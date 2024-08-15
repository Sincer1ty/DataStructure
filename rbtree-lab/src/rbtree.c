#include "rbtree.h"

#include <stdlib.h>

rbtree *new_rbtree(void)
{
	rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
	// TODO: initialize struct if needed
	node_t *n = (node_t *)calloc(1, sizeof(node_t));

	n->color = RBTREE_BLACK;

	n->parent = n;
	n->left = n;
	n->right = n;

	p->nil = n;
	p->root = p->nil;

	return p;
}

void delete_rbtree(rbtree *t)
{
	// TODO: reclaim the tree nodes's memory
	node_t *n = t->root;
	node_t *pre = t->nil;

	while (n != t->nil)
	{
		pre = n;
		if (n->left != t->nil)
			n = n->left;
		else
			n = n->right;
	}
	n = pre->parent;
	if (pre == n->left)
		n->left = t->nil;
	else
		n->right = t->nil;
	if (t->root != t->nil)
	{
		free(pre);

		if (n != t->nil)
		{
			return delete_rbtree(t);
		}
	}

	free(t->nil);
	free(t);
}

void left_rotate(rbtree *t, node_t *x)
{
	node_t *y = x->right;
	x->right = y->left;
	if (y->left != t->nil)
		y->left->parent = x;
	y->parent = x->parent;
	if (x->parent == t->nil)
		t->root = y;
	else if (x == x->parent->left)
		x->parent->left = y;
	else
		x->parent->right = y;
	y->left = x;
	x->parent = y;
}

void right_rotate(rbtree *t, node_t *x)
{
	node_t *y = x->left;
	x->left = y->right;
	if (y->right != t->nil)
		y->right->parent = x;
	y->parent = x->parent;
	if (x->parent == t->nil)
		t->root = y;
	else if (x == x->parent->left)
		x->parent->left = y;
	else
		x->parent->right = y;
	y->right = x;
	x->parent = y;
}

void rb_insert_fixup(rbtree *t, node_t *z)
{
	while (z->parent->color == RBTREE_RED) // 특성 4 위반
	{
		node_t *y; // 삼촌 노드

		if (z->parent == z->parent->parent->left)
		{
			y = z->parent->parent->right;

			if (y->color == RBTREE_RED)
			{
				z->parent->color = RBTREE_BLACK;
				y->color = RBTREE_BLACK;
				z->parent->parent->color = RBTREE_RED;
				z = z->parent->parent;
			}
			else
			{
				if (z == z->parent->right) // 좌회전
				{
					z = z->parent;
					left_rotate(t, z);
				}
				z->parent->color = RBTREE_BLACK;
				z->parent->parent->color = RBTREE_RED;
				right_rotate(t, z->parent->parent);
			}
		}
		else
		{
			y = z->parent->parent->left;

			if (y->color == RBTREE_RED)
			{
				z->parent->color = RBTREE_BLACK;
				y->color = RBTREE_BLACK;
				z->parent->parent->color = RBTREE_RED;
				z = z->parent->parent;
			}
			else
			{
				if (z == z->parent->left) // 우회전
				{
					z = z->parent;
					right_rotate(t, z);
				}
				z->parent->color = RBTREE_BLACK;
				z->parent->parent->color = RBTREE_RED;
				left_rotate(t, z->parent->parent);
			}
		}
	}
	t->root->color = RBTREE_BLACK; // z가 root일 경우 해결
}

node_t *rbtree_insert(rbtree *t, const key_t key)
{
	// TODO: implement insert
	node_t *z = (node_t *)calloc(1, sizeof(node_t));
	z->key = key;

	node_t *y = t->nil;
	node_t *x = t->root; // y의 자식
	while (x != t->nil)
	{
		y = x;
		if (z->key < x->key)
			x = x->left;
		else
			x = x->right;
	}
	z->parent = y;
	if (y == t->nil)
		t->root = z;
	else if (z->key < y->key)
		y->left = z;
	else
		y->right = z;
	z->left = t->nil;
	z->right = t->nil;
	z->color = RBTREE_RED;
	rb_insert_fixup(t, z);
	return t->root;
}

node_t *rbtree_find(const rbtree *t, const key_t key)
{
	// TODO: implement find
	node_t *n = t->root;

	while (n != t->nil)
	{
		if (key < n->key)
			n = n->left;
		else if (key > n->key)
			n = n->right;
		else
			return n;
	}

	return NULL;
}

node_t *rbtree_min(const rbtree *t)
{
	// TODO: implement find
	node_t *n = t->root;
	node_t *pre;

	while (n != t->nil)
	{
		pre = n;
		n = n->left;
	}

	return pre;
}

node_t *rbtree_max(const rbtree *t)
{
	// TODO: implement find
	node_t *n = t->root;

	while (n->right != t->nil)
	{
		n = n->right;
	}

	return n;
}

void rb_transplant(rbtree *t, node_t *u, node_t *v)
{
	if (u->parent == t->nil)
		t->root = v;
	else if (u == u->parent->left)
		u->parent->left = v;
	else
		u->parent->right = v;
	v->parent = u->parent;
}

node_t *tree_minimum(rbtree *t, node_t *root)
{
	node_t *n = root;

	while (n->left != t->nil)
	{
		n = n->left;
	}

	return n;
}

void rb_erase_fixup(rbtree *t, node_t *x)
{
	while (x != t->root && x->color == RBTREE_BLACK)
	{
		node_t *w; // 형제 노드
		if (x == x->parent->left)
		{
			w = x->parent->right;
			if (w->color == RBTREE_RED)
			{
				w->color = RBTREE_BLACK;
				x->parent->color = RBTREE_RED;
				left_rotate(t, x->parent);
				w = x->parent->right;
			}
			if (w->left->color == RBTREE_BLACK && w->right->color == RBTREE_BLACK)
			{
				w->color = RBTREE_RED;
				x = x->parent;
			}
			else
			{
				if (w->right->color == RBTREE_BLACK)
				{
					w->left->color = RBTREE_BLACK;
					w->color = RBTREE_RED;
					right_rotate(t, w);
					w = x->parent->right;
				}
				w->color = x->parent->color;
				x->parent->color = RBTREE_BLACK;
				w->right->color = RBTREE_BLACK;
				left_rotate(t, x->parent);
				x = t->root;
			}
		}
		else
		{
			w = x->parent->left;
			if (w->color == RBTREE_RED)
			{
				w->color = RBTREE_BLACK;
				x->parent->color = RBTREE_RED;
				right_rotate(t, x->parent);
				w = x->parent->left;
			}
			if (w->left->color == RBTREE_BLACK && w->right->color == RBTREE_BLACK)
			{
				w->color = RBTREE_RED;
				x = x->parent;
			}
			else
			{
				if (w->left->color == RBTREE_BLACK)
				{
					w->right->color = RBTREE_BLACK;
					w->color = RBTREE_RED;
					left_rotate(t, w);
					w = x->parent->left;
				}
				w->color = x->parent->color;
				x->parent->color = RBTREE_BLACK;
				w->left->color = RBTREE_BLACK;
				right_rotate(t, x->parent);
				x = t->root;
			}
		}
	}
	x->color = RBTREE_BLACK;
}

int rbtree_erase(rbtree *t, node_t *p)
{
	// TODO: implement erase
	node_t *y = p;
	color_t yOriginColor = y->color;

	node_t *x; // y의 자식
	if (p->left == t->nil)
	{
		x = p->right;
		rb_transplant(t, p, p->right);
	}
	else if (p->right == t->nil)
	{
		x = p->left;
		rb_transplant(t, p, p->left);
	}
	else
	{
		y = tree_minimum(t, p->right);
		yOriginColor = y->color;
		x = y->right;
		if (y->parent == p)
			x->parent = y; // x가 t->nil 일 수 있어서
		else
		{
			rb_transplant(t, y, y->right);
			y->right = p->right;
			y->right->parent = y;
		}
		rb_transplant(t, p, y);
		y->left = p->left;
		y->left->parent = y;
		y->color = p->color;
	}
	if (yOriginColor == RBTREE_BLACK)
		rb_erase_fixup(t, x);

	free(p);
	return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n)
{
	// TODO: implement to_array
	if (n == 0)
		return 0;

	node_t *node = t->root;
	node_t *visit = t->nil;
	int i = 0;

	while (node != t->nil && i < n)
	{
		if (visit == node->parent){
			if (node->left != t->nil){
				visit = node;
				node = node->left;
			}
			else {
				*(arr+i) = node->key;
				i++;
				visit = node;
				node = (node->right != t->nil) ? node->right : node->parent;
			}
		}
		else if (visit == node->left)
		{
			*(arr+i) = node->key;
			i++;
			visit = node;
			node = (node->right != t->nil) ? node->right : node->parent;
		}
		else {
			visit = node;
			node = node->parent;
		}
	}

	return 0;
}
