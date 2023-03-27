// Copyright 2021 Moscalu Cosmin-Andrei
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "hashtable.h"

#define MAX_BUCKET_SIZE 64

unsigned int hash_function_servers(void *a)
{
	unsigned int uint_a = *((unsigned int *)a);

	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
	uint_a = (uint_a >> 16u) ^ uint_a;
	return uint_a;
}

unsigned int hash_function_key(void *a)
{
	unsigned char *puchar_a = (unsigned char *) a;
	unsigned int hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c;

	return hash;
}

int compare_function_ints(void *a, void *b)
{
	int int_a = *((int *)a);
	int int_b = *((int *)b);

	if (int_a == int_b) {
		return 0;
	} else if (int_a < int_b) {
		return -1;
	} else {
		return 1;
	}
}

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}


hashtable_t* ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		        int (*compare_function)(void*, void*))
{
	hashtable_t *ht;
	unsigned int i, j;

	if (hmax <= 0)
		return NULL;

	ht = malloc(sizeof(*ht));
	if (!ht)
		return NULL;

	ht->hash_function = hash_function;
	ht->compare_function = compare_function;

	ht->hmax = hmax;
	ht->size = 0;

	ht->buckets = malloc(sizeof(*ht->buckets) * ht->hmax);
	if (!ht->buckets) {
		free(ht);
		return NULL;
	}

	for (i = 0; i < ht->hmax; ++i) {
		ht->buckets[i] = ll_create(sizeof(struct info));
		if (!ht->buckets[i]) {
			for (j = 0; j < i; ++j)
				free(ht->buckets[j]);
			free(ht->buckets);
			free(ht);
			return NULL;
		}
	}

	return ht;
}

void ht_put(hashtable_t *ht, void *key, unsigned int key_size, void *value,
	    unsigned int value_size)
{
	unsigned int index = ht->hash_function(key) % ht->hmax, i;
	struct info *hinfo = malloc(sizeof(*hinfo));
	struct info *tmp;
	ll_node_t *node;

	if (!hinfo)
		return;
	hinfo->key = malloc(key_size);
	if (!hinfo->key) {
		free(hinfo);
		return;
	}
	memcpy(hinfo->key, key, key_size);

	hinfo->value = malloc(value_size);
	if (!hinfo->value) {
		free(hinfo->key);
		free(hinfo);
		return;
	}
	memcpy(hinfo->value, value, value_size);

	ht->size++;

	if (ht->buckets[index]->size == 0) {
		ll_add_nth_node(ht->buckets[index], 0, hinfo);
		free(hinfo);
	} else {
		node = ht->buckets[index]->head;
		for (i = 0; i < ht->buckets[index]->size; ++i) {
			tmp = node->data;
			if (!ht->compare_function(key, tmp->key)) {
				free(tmp->value);
				if (!tmp->value) {
					free(hinfo->value);
					free(hinfo->key);
					free(hinfo);
					return;
				}
				tmp->value = hinfo->value;
				free(hinfo->key);
				free(hinfo);
				ht->size--;
				return;
			}
			node = node->next;
		}
		ll_add_nth_node(ht->buckets[index], ht->buckets[index]->size, hinfo);
		free(hinfo);
	}
}

void* ht_get(hashtable_t *ht, void *key)
{
	unsigned int index = ht->hash_function(key) % ht->hmax, i;
	ll_node_t *node = ht->buckets[index]->head;
	struct info *tmp;

	if (!node)
		return node;

	for (i = 0; i < ht->buckets[index]->size; ++i) {
		tmp = node->data;
		if (!ht->compare_function(key, tmp->key))
			return tmp->value;
		node = node->next;
	}

	return NULL;
}

int ht_has_key(hashtable_t *ht, void *key)
{
	unsigned int index = ht->hash_function(key) % ht->hmax, i;
	ll_node_t *node = ht->buckets[index]->head;
	struct info *tmp;

	if (!node)
		return 0;

	for (i = 0; i < ht->buckets[index]->size; ++i) {
		tmp = node->data;
		if (!ht->compare_function(key, tmp->key))
			return 1;
		node = node->next;
	}

	return 0;
}

void ht_remove_entry(hashtable_t *ht, void *key)
{
	unsigned int index = ht->hash_function(key) % ht->hmax, i;
	ll_node_t *node = ht->buckets[index]->head;
	struct info *tmp;

	if (!node)
		return;

	for (i = 0; i < ht->buckets[index]->size; ++i) {
		tmp = node->data;
		if (!ht->compare_function(key, tmp->key)) {
			node = ll_remove_nth_node(ht->buckets[index], i);
			tmp = node->data;

			free(tmp->value);
			free(tmp->key);
			free(tmp);
			free(node);

			ht->size--;

			return;
		}
		node = node->next;
	}
}

void ht_free(hashtable_t *ht)
{
	unsigned int i;
	ll_node_t *node, *next;
	struct info *tmp;
	for (i = 0; i < ht->hmax; ++i) {
		node = ht->buckets[i]->head;
		for (; node; ) {
			next = node->next;
			tmp = node->data;

			free(tmp->value);
			free(tmp->key);
			free(tmp);
			free(node);

			node = next;
		}
		free(ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
}

unsigned int ht_get_size(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->size;
}

unsigned int ht_get_hmax(hashtable_t *ht)
{
	if (ht == NULL)
		return 0;

	return ht->hmax;
}
