/* Copyright 2021 Moscalu Cosmin-Andrei */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

#define MAX_PRODUCTS 10000

server_memory* init_server_memory()
{
	server_memory *server = malloc(sizeof(*server));
	DIE(!server, "Failed to allocate memory for a server!");

	server->ht = ht_create(MAX_PRODUCTS, hash_function_key,
			       compare_function_strings);
	DIE(!server->ht, "Failed to allocate memory for a hashtable!");

	return server;
}

void server_store(server_memory *server, char *key, char *value)
{
	if (!server || !key || !value)
		return;

	ht_put(server->ht, key, strlen(key) + 1, value, strlen(value) + 1);
}

void server_remove(server_memory *server, char *key)
{
	if (!server || !key)
		return;

	ht_remove_entry(server->ht, key);
}

char* server_retrieve(server_memory *server, char *key)
{
	if (!server || !key)
		return NULL;

	return ht_get(server->ht, key);
}

void free_server_memory(server_memory *server)
{
	if (!server)
		return;

	ht_free(server->ht);
	free(server);
}
