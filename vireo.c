/* vireo - Preemptible green threads in C for Linux
 * https://github.com/geofft/vireo
 *
 * Copyright (c) 2014  Alexander Chernyakhovsky <achernya@mit.edu>
 * Copyright (c) 2014  Geoffrey Thomas <geofft@ldpreload.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/

#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/mman.h>

#include "vireo.h"

#define UNUSED __attribute__((unused))

// Environment structure, containing a status and a jump buffer.
typedef struct Env {
	int status;
	ucontext_t state;
	int state_reentered;
	int ipc_sender;
	int ipc_value;
} Env;

// Increase NENV to get more greenlets
#define NENV         1024

// Status codes for the Env
#define ENV_UNUSED    0
#define ENV_RUNNABLE  1
#define ENV_WAITING   2

#define ENV_STACK_SIZE  16384

static Env envs[NENV];
static int curenv;

void umain(void); /* provided by user */

/* Define a "successor context" for the purpose of calling env_exit */
static ucontext_t exiter = {0};

static void
make_stack(ucontext_t *ucp)
{
	// Reuse existing stack if any
	if (ucp->uc_stack.ss_sp)
		return;

	ucp->uc_stack.ss_sp = mmap(
		NULL, ENV_STACK_SIZE, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_GROWSDOWN | MAP_PRIVATE,
		-1, 0);
	ucp->uc_stack.ss_size = ENV_STACK_SIZE;
}

int
vireo_create(vireo_entry entry)
{
	// Find an available environment
	int env;
	for (env = 0; (env < NENV); env++) {
		if (envs[env].status == ENV_UNUSED) {
			// This one is usable!
			break;
		}
	}
	if (env == NENV) {
		// No available environments found
		return -1;
	}
	envs[env].status = ENV_RUNNABLE;

	getcontext(&envs[env].state);
	make_stack(&envs[env].state);
	envs[env].state.uc_link = &exiter;
	makecontext(&envs[env].state, entry, 0);
	// Creation worked. Yay.
	return env;
}

static void
vireo_schedule(void)
{
	int attempts = 0;
	int candidate;
	while (attempts < NENV) {
		candidate = (curenv + attempts + 1) % NENV;
		if (envs[candidate].status == ENV_RUNNABLE) {
			curenv = candidate;
			setcontext(&envs[curenv].state);
		}
		attempts++;
	}
	exit(0);
}

void
vireo_yield(void)
{
	envs[curenv].state_reentered = 0;
	getcontext(&envs[curenv].state);
	if (envs[curenv].state_reentered++ == 0) {
		// Save successful, find the next process to run.
		vireo_schedule();
	}
	// We've re-entered. Do nothing.
}

void
vireo_exit(void)
{
	envs[curenv].status = ENV_UNUSED;
	vireo_schedule();
}

void
vireo_destroy(int env)
{
	envs[env].status = ENV_UNUSED;
}

int
vireo_getid(void)
{
	return curenv;
}

int
vireo_recv(int *who)
{
	envs[curenv].status = ENV_WAITING;
	vireo_yield();
	if (who)
		*who = envs[curenv].ipc_sender;
	return envs[curenv].ipc_value;
}

void
vireo_send(int toenv, int val)
{
	while (envs[toenv].status != ENV_WAITING)
		vireo_yield();
	envs[toenv].ipc_sender = curenv;
	envs[toenv].ipc_value = val;
	envs[toenv].status = ENV_RUNNABLE;
}

static void
initialize_threads(vireo_entry new_main)
{
	curenv = 0;

	getcontext(&exiter);
	make_stack(&exiter);
	makecontext(&exiter, vireo_exit, 0);

	vireo_create(new_main);
	setcontext(&envs[curenv].state);
}

int
main(int argc UNUSED, char* argv[] UNUSED)
{
	initialize_threads(umain);
	return 0;
}
