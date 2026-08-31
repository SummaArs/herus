#include <stdint.h>
#include <stdio.h>

#define MAX_NODES 8u

typedef enum {
    OP_VAR = 0,
    OP_ADD = 1,
    OP_MUL = 2,
    OP_NEG = 3
} Op;

typedef struct {
    uint8_t op;
    uint8_t left;
    uint8_t right;
} Node;

static Node nodes[MAX_NODES];
static uint8_t node_count;

static int64_t eval(uint8_t index, int64_t x) {
    const Node node = nodes[index];
    switch (node.op) {
        case OP_VAR: return x;
        case OP_ADD: return eval(node.left, x) + eval(node.right, x);
        case OP_MUL: return eval(node.left, x) * eval(node.right, x);
        case OP_NEG: return -eval(node.left, x);
        default: return 0;
    }
}

static int append(Op op, uint8_t left, uint8_t right) {
    if (node_count >= MAX_NODES) return 0;
    nodes[node_count] = (Node){(uint8_t)op, left, right};
    ++node_count;
    return 1;
}

int main(void) {
    /* 0:x, 1:x*x, 2:-x, 3:x*x-x, 4:x+x, 5:(x*x-x)*(x+x). */
    if (!append(OP_VAR, 0u, 0u) ||
        !append(OP_MUL, 0u, 0u) ||
        !append(OP_NEG, 0u, 0u) ||
        !append(OP_ADD, 1u, 2u) ||
        !append(OP_ADD, 0u, 0u) ||
        !append(OP_MUL, 3u, 4u)) {
        return 2;
    }

    const int64_t samples[] = {-2, -1, 0, 1, 2};
    for (uint8_t i = 0u; i < (uint8_t)(sizeof(samples) / sizeof(samples[0])); ++i) {
        const int64_t x = samples[i];
        const int64_t expected = (x * x - x) * (x + x);
        if (eval(5u, x) != expected) return 3;
    }

    printf("nodes=%u root=5 verified=1 static_bytes=%zu node_bytes=%zu\n",
           (unsigned)node_count, sizeof(nodes) + sizeof(node_count), sizeof(nodes));
    return 0;
}
