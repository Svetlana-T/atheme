/*
 * Copyright (c) 2006 Atheme Development Group
 * Rights to this code are as documented in doc/LICENSE.
 *
 * PLAIN mechanism provider
 */

#include "atheme.h"

static const sasl_core_functions_t *sasl_core_functions = NULL;

static int
mech_step(sasl_session_t *const restrict p, char *const restrict message, const size_t len,
          char __attribute__((unused)) **const restrict out,
          size_t __attribute__((unused)) *const restrict out_len)
{
	if (! (message && len))
		return ASASL_FAIL;

	char data[768];
	if (len >= sizeof data)
		return ASASL_FAIL;

	(void) memset(data, 0x00, sizeof data);
	(void) memcpy(data, message, len);

	const char *ptr = data;
	const char *const end = data + len;

	const char *const authzid = ptr;
	if (! *ptr || (ptr += strlen(ptr) + 1) >= end)
		return ASASL_FAIL;

	const char *const authcid = ptr;
	if (! *ptr || (ptr += strlen(ptr) + 1) >= end)
		return ASASL_FAIL;

	const char *const secret = ptr;
	if (! *secret)
		return ASASL_FAIL;

	myuser_t *const mu = myuser_find_by_nick(authcid);
	if (! mu)
		return ASASL_FAIL;

	if (mu->flags & MU_NOPASSWORD)
		return ASASL_FAIL;

	p->username = sstrdup(authcid);
	p->authzid = sstrdup(authzid);

	if (! verify_password(mu, secret))
		return ASASL_FAIL;

	return ASASL_DONE;
}

static sasl_mechanism_t mech = {

	.name           = "PLAIN",
	.mech_start     = NULL,
	.mech_step      = &mech_step,
	.mech_finish    = NULL,
};

static void
mod_init(module_t *const restrict m)
{
	MODULE_TRY_REQUEST_SYMBOL(m, sasl_core_functions, "saslserv/main", "sasl_core_functions");

	(void) sasl_core_functions->mech_register(&mech);
}

static void
mod_deinit(const module_unload_intent_t __attribute__((unused)) intent)
{
	(void) sasl_core_functions->mech_unregister(&mech);
}

SIMPLE_DECLARE_MODULE_V1("saslserv/plain", MODULE_UNLOAD_CAPABILITY_OK)
