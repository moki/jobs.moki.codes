import React, { HTMLProps } from "react";

import "./index.css";

import { TagProp } from "src/components/component";

export const HeadingLevels = [1, 2, 3 /*, 4, 5, 6*/] as const;

export type LevelProp = {
    level: typeof HeadingLevels[number];
};

export type HeadingProps = TagProp & LevelProp & HTMLProps<HTMLElement>;

export function Heading({
    tag,
    level,
    className,
    children,
    ...rest
}: HeadingProps) {
    const classes =
        `heading heading_level_${level}` + (className ? " " + className : "");

    return React.createElement(tag, { ...rest, className: classes }, children);
}
