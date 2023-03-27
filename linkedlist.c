// Copyright 2021 Moscalu Cosmin-Andrei
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linkedlist.h"
#include "utils.h"

linked_list_t* ll_create(unsigned int data_size)
{
	linked_list_t *list;
	list = malloc(sizeof(*list));
	if (!list)
		return NULL;
	list->head = NULL;
	list->size = 0;
	list->data_size = data_size;
	return list;
}

void ll_add_nth_node(linked_list_t* list, unsigned int n, const void* new_data)
{
	ll_node_t *node, *current;
	unsigned int i;

	if (!list)
		return;

	node = malloc(sizeof(*node));
	if (!node)
		return;

	node->data = malloc(list->data_size);
	if (!node->data)
		return;

	memcpy(node->data, new_data, list->data_size);

	if (n == 0 || list->size == 0) {
		node->next = list->head;
		list->head = node;
		list->size++;
		return;
	}

	current = list->head;
	for (i = 0; i < n - 1; ++i) {
		if (!current->next)
			break;
		current = current->next;
	}

	node->next = current->next;
	current->next = node;
	list->size++;
}

ll_node_t* ll_remove_nth_node(linked_list_t* list, unsigned int n)
{
	ll_node_t *current, *next;
	unsigned int i;

	if (!list)
		return NULL;
	if (!list->size)
		return NULL;

	if (n == 0 || list->size == 1) {
		current = list->head;
		list->head = current->next;
		list->size--;
		return current;
	}

	current = list->head;
	next = current->next;
	for (i = 0; i < n - 1; ++i) {
		if (!current->next)
			break;
		current = next;
		next = current->next;
	}

	current->next = next->next;
	list->size--;
	return next;
}

unsigned int ll_get_size(linked_list_t* list)
{
	if (!list)
		return -1;
	return list->size;
}

void ll_free(linked_list_t** pp_list)
{
	ll_node_t *next, *current;

	if (!pp_list || !*pp_list)
		return;

	current = (*pp_list)->head;
	for (; current != NULL; ) {
		next = current->next;
		free(current->data);
		free(current);
		current = next;
	}
	free(*pp_list);
	*pp_list = NULL;
}
