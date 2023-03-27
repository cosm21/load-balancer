// Copyright 2021 Moscalu Cosmin-Andrei
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ocdll.h"
#include "utils.h"

ocdll_t* ocdll_create(unsigned int data_size,
		      int (*cmp_func)(const void*, const void*))
{
	ocdll_t *list = malloc(sizeof(*list));
	if (!list)
		return list;
	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;
	list->cmp_func = cmp_func;
	return list;
}

/*
 * Intoarce nodul care are informatiile specificate (node->data), altfel
 * intoarce NULL.
 */
ocdll_node_t* ocdll_get_exact(ocdll_t *list, const void *data)
{
	ocdll_node_t *node;
	int ret;

	if (!list || !list->size || !data)
		return NULL;

	for (node = list->head;; node = node->next) {
		ret = list->cmp_func(data, node->data);
		if (ret == 0)
			return node;
		else if (ret < 0)
			return NULL;
	}
}

/*
 * Intoarce primul nod din lista care are datele (node->data) "mai mari"
 * decat cele date ca parametri.
 */
ocdll_node_t* ocdll_get(ocdll_t *list, const void *data)
{
	ocdll_node_t *node;

	if (!list || !list->size || !data)
		return NULL;
	/*
	 * Parcurge lista, pana cand gaseste nodul care se potriveste cerintei,
	 * sau pana cand ajunge la finalul listei
	 */
	for (node = list->head; list->cmp_func(data, node->data) >= 0 &&
	     node != list->head->prev; node = node->next) {}

	/*
	 * Daca am ajuns la finalul listei, atunci verific daca datele
	 * se afla "inaintea" sau "dupa" cele ale ultimului nod
	 */
	if (node == list->head->prev && list->cmp_func(data, node->data) >= 0)
		return node->next;

	return node;
}

/*
 * Insereaza datele in lista, pastrand ordinea acesteia.
 */
void ocdll_insert(ocdll_t* list, const void* data)
{
	ocdll_node_t *node, *new;

	if (!list || !data)
		return;

	new = malloc(sizeof(*new));
	if (!new)
		return;
	new->data = malloc(list->data_size);
	if (!new->data) {
		free(new);
		return;
	}
	memcpy(new->data, data, list->data_size);

	if (!list->size) {
		list->head = new;
		new->prev = new;
		new->next = new;
	} else {
		node = ocdll_get(list, data);
		node = node->prev;
		new->next = node->next;
		new->prev = node;
		node->next->prev = new;
		node->next = new;
		/*
		 * Daca noul nod este "mai mic" decat toate celelalte
		 * atunci devine noul inceput al listei.
		 */
		if (new->next == list->head &&
		    list->cmp_func(data, new->next->data) < 0) {
			list->head = new;
		}
	}
	list->size++;
}

/*
 * Elibereaza nodul care are exact datele specificate.
 */
ocdll_node_t* ocdll_remove(ocdll_t* list, const void *data)
{
	ocdll_node_t *node, *next, *ret;

	if (!list || !list->size)
		return NULL;

	node = ocdll_get_exact(list, data);
	if (!node)
		return NULL;
	ret = node;
	node = node->prev;
	next = node->next->next;
	next->prev = node;
	node->next = next;

	if (!list->cmp_func(ret->data, list->head->data))
		list->head = next;

	if (list->size == 1)
		list->head = NULL;

	list->size--;
	return ret;
}

unsigned int ocdll_get_size(ocdll_t* list)
{
	return list->size;
}

void ocdll_free(ocdll_t** pp_list, void (*free_data_func)(void*))
{
	ocdll_node_t *node, *next;
	unsigned int i;

	if (!pp_list || !*pp_list)
		return;

	node = (*pp_list)->head;
	for (i = 0; i < (*pp_list)->size; ++i) {
		next = node->next;
		free_data_func(node->data);
		free(node);
		node = next;
	}

	free(*pp_list);
	*pp_list = NULL;
}
