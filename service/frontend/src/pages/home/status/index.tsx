import React, { HTMLProps, ReactNode } from "react";

import { Section } from "src/components/section";
import { Container } from "src/components/container";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";
import { Text } from "src/components/text";

import "./index.css";

export function Status() {
    return (
        <Section.Component>
            <Container.Component>
                <Heading tag="h3" level={2}>
                    Status
                </Heading>
                <SubHeading tag="p" level={2}>
                    tracking progress
                </SubHeading>
                <Text.Component tag="p">
                    Progress so far and plans for the future.
                </Text.Component>
                <Implemented />
                <Unimplemented />
            </Container.Component>
        </Section.Component>
    );
}

function Implemented() {
    const classes = "status__implemented";

    const is = [
        "skills",
        [
            "dominance",
            ["dataset querying", ["top filters", "individual skill"]],
            ["tooltip"],
        ],
        "salaries",
        ["distribution", ["box plot", "violin plot"], ["tooltip"]],
    ];

    return (
        <div className={classes}>
            <Heading tag="h4" level={3}>
                Implemented
            </Heading>
            <SubHeading tag="p" level={3}>
                features
            </SubHeading>
            <StatusList items={is} />
        </div>
    );
}

function Unimplemented() {
    const classes = "status__unimplemented";

    const us = [
        "skills",
        [
            "dominance",
            [
                "dataset querying",
                ["exclude skill", "presets e.g: backend/frontend skills"],
            ],
            "chords",
        ],
        "dataset",
        ["block—list", "merge—list"],
    ];

    return (
        <div className={classes}>
            <Heading tag="h4" level={3}>
                Unimplemented
            </Heading>
            <SubHeading tag="p" level={3}>
                features
            </SubHeading>
            <StatusList items={us} />
        </div>
    );
}

type StatusListItemProps = {
    depth: number;
} & HTMLProps<HTMLLIElement>;

function StatusListItem({ depth, children }: StatusListItemProps) {
    const classes = "status-list__item";

    return (
        <li
            className={classes}
            style={{ paddingLeft: `calc(var(--gc) * 5 * ${depth})` }}
        >
            &#8226; {children}
        </li>
    );
}

type Item = string | string[] | Items;

type Items = Item[];

type StatusListProps = {
    items: Items;
} & HTMLProps<HTMLUListElement>;

function StatusList({ items }: StatusListProps) {
    const classes = "status__list";

    const render = (items: Items, depth = 0): ReactNode =>
        items.map((s, i: number) =>
            Array.isArray(s) ? (
                render(s, depth + 1)
            ) : (
                <StatusListItem depth={depth} key={depth + "" + i}>
                    {s}
                </StatusListItem>
            )
        );

    return <ul className={classes}>{render(items)}</ul>;
}
