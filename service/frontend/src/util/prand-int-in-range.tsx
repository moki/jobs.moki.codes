export function prandIntInRange(left: number, right: number) {
    return Math.floor(Math.random() * (right - left + 1)) + left;
}
