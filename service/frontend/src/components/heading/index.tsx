import { h } from "preact";

import "./index.css";

import {
    TagProp,
    ChildrenProp,
    HTMLAttributes,
} from "src/components/component";

export const HeadingLevels = [1, 2 /*, 3, 4, 5, 6*/] as const;

export type LevelProp = {
    level: typeof HeadingLevels[number];
};

export type HeadingProps = TagProp & ChildrenProp & LevelProp & HTMLAttributes;

export function Heading({ tag, level, children, ...rest }: HeadingProps) {
    const cs =
        `heading heading_level_${level}` + (rest.class ? " " + rest.class : "");

    return h(tag, { ...rest, class: cs }, children);
}
