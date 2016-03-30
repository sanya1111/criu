#ifndef PSTREE_ZDTMTST_LIST_H_
#define PSTREE_ZDTMTST_LIST_H_

struct __children_list_entry {
	pid_t * pid;
	struct __children_list_entry * next;
};

#define __children_list_entry_for(var) \
	__children_list_entry_##var

#define __children_list_entry_init(var) \
	__children_list_entry_for(var) = { \
		.pid = &(var), \
		.next = &(__children_list_entry_for(var) ) \
	}

#define __children_list_entry_create(var) \
	struct __children_list_entry __children_list_entry_init(var);

#define __children_list_clean \
	__children_list_entry_for(head).next = &(__children_list_entry_for(head) );

#define __children_list_create \
	struct __children_list_entry __children_list_entry_for(head);\
	__children_list_clean;

#define __children_list_add_entry(new) \
	__children_list_entry_for(new).next = __children_list_entry_for(head).next; \
	__children_list_entry_for(head).next = &(__children_list_entry_for(new));

#define __children_list_for_each(ptr) \
	for (ptr = __children_list_entry_for(head).next; ptr != &(__children_list_entry_for(head)); ptr = ptr->next)

#endif /* PSTREE_ZDTMTST_LIST_H_ */
