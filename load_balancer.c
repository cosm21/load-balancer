/* Copyright 2021 Moscalu Cosmin-Andrei */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"
#include "utils.h"
#include "ocdll.h"

#define MAX_SERVERS 100000
#define MAX_REPLICAS 2

struct load_balancer {
	ocdll_t *list;
};

/*
 * Structura de date care tine informatii despre un server sau
 * o replica a unui server pe hashring, fiecare replica a serverului
 * are un pointer la acelasi server pentru a evita dublarea.
 */
struct server_info {
	unsigned int sid;
	unsigned int rid;
	unsigned int tag;
	unsigned int hash;
	server_memory *server;
};

/*
 * Compara doua structuri care contin informatii despre un server, intorcand
 * un numar pozitiv daca a are hash-ul mai mare decat b, 0 daca au hash-ul
 * egal, si un numar negativ altfel. Este folosita pentru a tine hash ring-ul
 * ordonat.
 */
int compare_function_sinfo(const void *a, const void *b)
{
	if (!a || !b)
		return 0;

	const server_info *sa = a;
	const server_info *sb = b;

	if (sa->hash > sb->hash)
		return 1;
	else if (sa->hash == sb->hash)
		return 0;
	else
		return -1;
}

load_balancer* init_load_balancer()
{
	load_balancer *load = malloc(sizeof(*load));
	DIE(!load, "Failed to allocate memory for the load balancer!");

	load->list = ocdll_create(sizeof(server_info), compare_function_sinfo);
	DIE(!load->list, "Failed to allocate memory for the list of servers!");

	return load;
}

void loader_store(load_balancer* main, char* key, char* value, int* server_id)
{
	ocdll_node_t *node;
	server_info sinfo_tmp, *sinfop;
	unsigned int hash;

	if (!main || !key || !value || !server_id)
		return;

	hash = hash_function_key(key);
	sinfo_tmp.hash = hash;
	/*
	 * Gasesc in hash ring primul server care are hash-ul mai mare
	 * decat cheia data.
	 */
	node = ocdll_get(main->list, &sinfo_tmp);

	if (!node)
		return;

	sinfop = node->data;

	*server_id = sinfop->sid;
	server_store(sinfop->server, key, value);
}

char* loader_retrieve(load_balancer* main, char* key, int* server_id)
{
	ocdll_node_t *node;
	server_info sinfo_tmp, *sinfop;
	unsigned int hash;

	if (!main || !key || !server_id)
		return NULL;

	hash = hash_function_key(key);
	sinfo_tmp.hash = hash;
	/*
	 * Daca cheia se afla in load balancer se va gasi in serverul cel mai
	 * apropriat.
	 */
	node = ocdll_get(main->list, &sinfo_tmp);

	if (!node) {
		return NULL;
	}

	sinfop = node->data;
	*server_id = sinfop->sid;
	return server_retrieve(sinfop->server, key);
}

/*
 * Functie care redistribuie (sau nu) perechile cheie valoare ale unui server
 * dat in alte servere din hash ring.
 */
void loader_redistribute_server(load_balancer *main, server_info *sinfop)
{
	unsigned int i;
	int sid = 0;
	ll_node_t *node;
	struct info *infop;
	server_memory *server;

	if (!main || !sinfop)
		return;

	server = sinfop->server;
	for (i = 0; i < server->ht->hmax; ++i) {
		for (node = server->ht->buckets[i]->head; node; node = node->next) {
			infop = node->data;
			loader_store(main, infop->key, infop->value, &sid);

			/*
			 * Daca serverul pe care il alegem pentru a stoca
			 * perechea este diferit de cel pe care il redistribuim
			 * inseamna ca o sa avem un duplicat pe serverul
			 * pe care era perechea original.
			 */
			if (sid != (int)sinfop->sid) {
				server_remove(server, infop->key);
				--i;
				break;
			}
		}
	}
}

void loader_add_server(load_balancer* main, int server_id)
{
	unsigned int i;
	ocdll_node_t *node;
	server_info sinfo_tmp, *sinfop;
	server_memory *server;

	if (!main)
		return;

	server = init_server_memory();

	/*
	 * Adaug serverul si replicile sale, calculand hash-ul.
	 */
	for (i = 0; i <= MAX_REPLICAS; ++i) {
		sinfo_tmp.sid = server_id;
		sinfo_tmp.rid = i;
		sinfo_tmp.tag = i * 100000 + server_id;
		sinfo_tmp.hash = hash_function_servers(&sinfo_tmp.tag);
		sinfo_tmp.server = server;

		ocdll_insert(main->list, &sinfo_tmp);
	}

	/*
	 * Pentru serverul nou adaugat si fiecare replica a sa, caut primul
	 * server care are hash-ul mai mare, si pentru serverul gasit
	 * redistribui valorile.
	 */
	for (i = 0; i <= MAX_REPLICAS; ++i) {
		sinfo_tmp.tag = i * 100000 + server_id;
		sinfo_tmp.hash = hash_function_servers(&sinfo_tmp.tag);

		node = ocdll_get(main->list, &sinfo_tmp);
		if (!node)
			return;

		sinfop = node->data;

		/*
		 * Daca serverul care urmeaza in hash ring este acelasi server
		 * atunci valorile au fost sau vor fi redistribuite oricum.
		 */
		if (sinfop->sid == sinfo_tmp.sid)
			continue;

		loader_redistribute_server(main, sinfop);
	}
}

void loader_remove_server(load_balancer* main, int server_id)
{
	unsigned int i, sid;
	ocdll_node_t *node;
	server_info sinfo_tmp, *sinfop;
	server_memory *server;

	if (!main)
		return;

	for (i = 0; i <= MAX_REPLICAS; ++i) {
		sinfo_tmp.tag = i * 100000 + server_id;
		sinfo_tmp.hash = hash_function_servers(&sinfo_tmp.tag);

		node = ocdll_remove(main->list, &sinfo_tmp);
		if (!node)
			return;

		sinfop = node->data;

		if (!sinfop->rid) {
			sid = sinfop->sid;
			server = sinfop->server;
		}

		free(sinfop);
		free(node);
	}

	sinfo_tmp.sid = sid;
	sinfo_tmp.server = server;
	/*
	 * Daca serverul a fost scos deja din hash ring atunci toate valorile
	 * tinute de acesta vor fi redistribuite.
	 */
	loader_redistribute_server(main, &sinfo_tmp);

	free_server_memory(server);
}

void free_sinfo(void *ptr)
{
	server_info *sinfop = ptr;
	if (!ptr)
		return;

	if (!sinfop->rid)
		free_server_memory(sinfop->server);
	free(sinfop);
}

void free_load_balancer(load_balancer* main)
{
	if (!main)
		return;

	ocdll_free(&main->list, free_sinfo);
	free(main);
}
