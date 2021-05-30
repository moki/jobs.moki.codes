import { h } from "preact";

import "./index.css";

import {
    TagProp,
    ChildrenProp,
    HTMLAttributes,
} from "src/components/component";

export const SubHeadingLevels = [1, 2 /*, 3, 4, 5, 6*/] as const;

export type LevelProp = {
    level: typeof SubHeadingLevels[number];
};

export type SubHeadingProps = TagProp &
    ChildrenProp &
    LevelProp &
    HTMLAttributes;

export function SubHeading({ tag, level, children, ...rest }: SubHeadingProps) {
    const cs =
        `sub-heading sub-heading_level_${level}` +
        (rest.class ? " " + rest.class : "");

    return h(tag, { ...rest, class: cs }, children);
}
