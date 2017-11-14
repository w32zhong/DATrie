#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 100
#define M (26 + 1)

#define MIN(x, y) ((x) < (y) ? (x) : (y))

int base[N];
int check[N];

void print()
{
	int i;
	printf("%6s", "state");
	printf("%6s", "base");
	printf("%6s", "check");
	printf("\n");

	for (i = 0; i < N; i++) {
		printf("[%3d]", i);
		printf(" %3d ", base[i]);
		printf(" %3d ", check[i]);
		printf("\n");
	}
}

int cmap(char c)
{
	return (int)(c - 'a' + 1);
}

int *children(int state)
{
	int *child = malloc(sizeof(int) * (M + 1 /*'\0'*/));
	int i, j = 0;
	for (i = 1; i < N; i++) {
		if (check[i] == state)
			child[j++] = i;
	}
	child[j] = 0;

	printf("children of [%d]: ", state);
	for (int i = 0; child[i] != 0; i++) {
		printf("[%d] ", child[i]);
	}
	printf("\n");

	return child;
}

int base_alloc(int *child, int conflict_state)
{
	int j, q = 0;
	do {
		q ++;
		for (j = 0; child[j] != 0; j++) {
			int new_state = q + child[j];
			if (new_state == conflict_state ||
			    base[new_state] != 0)
				break;
		}
		if (child[j] == 0)
			break;
	} while (1);

	return q;
}

int relocate(int state, int *child, int new_base)
{
	int *grand_child, j, k;
	for (j = 0; child[j] != 0; j++) {
		int new_state = new_base + child[j] - base[state];

		printf("copy child [%d] to new state -->  %d \n", child[j], new_state);
		base[new_state] = base[child[j]]; // copy child base value
		check[new_state] = state;  // set new child parent
		base[child[j]] = 0; // clean old child base value
		check[child[j]] = 0; // clean old child parent

		// correct grand children CHECKs.
		grand_child = children(child[j]);

		for (k = 0; grand_child[k] != 0; k++) {
			printf("correct CHECK[%d] --> %d \n",
			       grand_child[k], new_state);
			check[grand_child[k]] = new_state;
		}
		free(grand_child);
	}

	printf("update BASE[%d] --> %d \n", state, new_base);
	base[state] = new_base;
	return new_base;
}

void resolve(int state, int conflict_state)
{
	int new_base, *child = children(state);
	new_base = base[state] + base_alloc(child, conflict_state);
	printf("Relocating state %d to new base %d \n", state, new_base);

	relocate(state, child, new_base);
	free(child);
}

int insert_char(int cur_state, int next_state)
{
	if (base[next_state] == 0) {
		base[next_state] = next_state; /* better hueristic value? TODO */
		check[next_state] = cur_state;

		printf("Insert [%d] \n", next_state);

	} else if (check[next_state] != cur_state) {
		printf("check[%d] != %d \n", next_state, cur_state);
		resolve(check[next_state], next_state);
		return 1;
	} else {
		printf("Walk: [%d] \n", next_state);
	}

	return 0;
}

int insert(char *str)
{
	int next_state, cur_state = 1;
	int c, str_idx = 0;

restart:
	while (str_idx < strlen(str)) {
		c = cmap(str[str_idx]);
		next_state = base[cur_state] + c;

		printf("For `%c' (+%d) \n", str[str_idx], c);

		if (next_state >= N) {
			printf("N too small !!! \n");
			return 1;
		}

		if (insert_char(cur_state, next_state) != 0) {
			/* conflict resolved, but we need to restart
			 * in case cur_state is changed */
			cur_state = 1;
			str_idx = 0;
			goto restart;
		}

		cur_state = next_state;
		str_idx ++;
	}

	return 0;
}

int lookup(char *str)
{
	int next_state, cur_state = 1;
	int c, str_idx = 0;

	while (str_idx < strlen(str)) {
		c = cmap(str[str_idx]);
		next_state = base[cur_state] + c;

		printf("`%c' (+%d), next state = %d \n", str[str_idx], c, next_state);

		if (base[next_state] == 0 || check[next_state] != cur_state)
			return 0;

		cur_state = next_state;
		str_idx ++;
	}

	return 1;
}

char test_string[][64] = {
	"bachelor",
	"jar",
	"air",
	"badge",
	"baby",
	"bad",
	"badly",
	"boy",
	"apple",
	"app"
};

int main()
{
	int i, ret = 0;

	/* array[0] does not matter since our root starts
	 * from array[1]. We avoid array[0] because finding
	 * children function will return unexpected children
	 * becuase every CHECK[i] is initialized with zero. */
	base[0] = 1234;
	check[0] = -4321;
	base[1] = 1;

	for (i = 0; i < sizeof(test_string) / 64; i++) {
		printf("Inserting %s ...\n", test_string[i]);
		ret |= insert(test_string[i]);
		printf("\n");
	}

	printf("Insertions return code: %x \n\n", ret);

	printf("=== Final Double Array === \n");
	print();
	printf("\n");

	for (i = 0; i < sizeof(test_string) / 64; i++) {
		ret = lookup(test_string[i]);
		printf("Look up return code for `%s': %d \n", test_string[i], ret);
	}
	return 0;
}
