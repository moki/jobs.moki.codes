import { zip } from "ramda";

export type Node<T> = {
    value?: T;
    children: { [key: string]: Node<T> };
};

function Node<T>(v?: T): Node<T> {
    return {
        value: v,
        children: {},
    };
}

type PathNode<T> = { key: string; node: Node<T> };

export type LookupResult<T> = { key: string; value: T };

export class Trie<T> {
    private root: Node<T>;

    public length: number;

    constructor() {
        this.length = 0;

        this.root = Node();
    }

    insert(k: string, v: T): Trie<T> {
        let cursor = this.root;

        for (let c of k)
            cursor = cursor.children[c] ?? (cursor.children[c] = Node());

        cursor.value = v;

        this.length++;

        return this;
    }

    lookup(k: string): LookupResult<T>[] {
        let cursor = this.root;

        for (let c of k) {
            cursor = cursor.children[c];

            if (!cursor) return [];
        }

        let matches = [];

        let stack = [{ key: k, node: cursor }];

        for (; stack.length; ) {
            const { key, node } = stack.pop() as PathNode<T>;

            stack.push(
                ...zip(
                    Object.keys(node.children),
                    Object.values(node.children)
                ).map(([k, v]) => ({ key: key + k, node: v }))
            );

            if (typeof node.value !== "undefined")
                matches.push({ key: key, value: node.value });
        }

        return matches;
    }
}
