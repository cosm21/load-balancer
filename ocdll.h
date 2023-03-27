// Copyright 2021 Moscalu Cosmin-Andrei
#ifndef OCDLL_H_
#define OCDLL_H_

typedef struct ocdll_node_t ocdll_node_t;
struct ocdll_node_t
{
	void* data;
	ocdll_node_t *prev, *next;
};

/*
 * Pe langa ce aveam din lista circulara dublu inlantuita, in ocdll
 * mai apare un pointer la o functie de comparare care este folosita
 * pentru a compara informatiile din noduri (node->data), astfel
 * lista este mereu ordonata
 */
typedef struct ocdll_t ocdll_t;
struct ocdll_t
{
	ocdll_node_t *head;
	unsigned int data_size;
	unsigned int size;
	int (*cmp_func)(const void*, const void*);
};

ocdll_t* ocdll_create(unsigned int data_size,
		      int (*cmp_func)(const void*, const void*));

ocdll_node_t* ocdll_get_exact(ocdll_t *list, const void *data);

ocdll_node_t* ocdll_get(ocdll_t *list, const void *data);

void ocdll_insert(ocdll_t *list, const void *data);

ocdll_node_t* ocdll_remove(ocdll_t *list, const void *data);

unsigned int ocdll_get_size(ocdll_t *list);

void ocdll_free(ocdll_t** pp_list, void (*free_func)(void*));

#endif  // OCDLL_H_
