import { prandIntInRange } from "src/util/prand-int-in-range";
import { zip, flatten } from "ramda";

export type Config = {
    hStep: number;
    minS: number;
    maxS: number;
    minL: number;
    maxL: number;
    minA: number;
    maxA: number;
};

const defaultConfig = {
    hStep: 222.5,
    minS: 75,
    maxS: 95,
    minL: 50,
    maxL: 55,
    minA: 70,
    maxA: 100,
};

export function palette(size: number, config: Config = defaultConfig) {
    let hs = new Set<number>();

    let ss = new Array<number>(size);

    let ls = new Array<number>(size);

    let vs = new Array<number>(size);

    let n = 0;

    let i = 0;

    for (; hs.size < size; i++) {
        let color = (n = n + config.hStep);

        for (; hs.has(color % size); ) color = n = n + config.hStep;

        hs.add(color);

        ss[i] = prandIntInRange(config.minS, config.maxS);

        ls[i] = prandIntInRange(config.minL, config.maxL);

        vs[i] = prandIntInRange(config.minA, config.maxA);
    }

    return zip(zip(zip(Array.from(hs), ss), ls), vs).reduce<
        [number, number, number, number][]
    >((a: any, x: any) => [...a, flatten(x)], []);
}

palette(10);
