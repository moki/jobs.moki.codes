import React, { HTMLProps } from "react";

import "./index.css";

import { TagProp } from "src/components/component";

export const SubHeadingLevels = [1, 2, 3 /*, 4, 5, 6*/] as const;

export type LevelProp = {
    level: typeof SubHeadingLevels[number];
};

export type SubHeadingProps = TagProp & LevelProp & HTMLProps<HTMLElement>;

export function SubHeading({
    tag,
    level,
    className,
    children,
    ...rest
}: SubHeadingProps) {
    const classes =
        `sub-heading sub-heading_level_${level}` +
        (className ? " " + className : "");

    return React.createElement(tag, { ...rest, className: classes }, children);
}
