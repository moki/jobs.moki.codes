export function zip<T1, T2>(arr: Array<T1>, ...arrs: Array<T2>) {
    return arr.map((val: T1, i: number) =>
        arrs.reduce((a: Array<T1>, arr: any) => [...a, arr[i]], [val])
    );
}
