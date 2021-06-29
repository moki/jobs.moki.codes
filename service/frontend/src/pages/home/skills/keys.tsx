import React from "react";

import { scaleOrdinal } from "@visx/scale";
import { LegendOrdinal, LegendItem, LegendLabel } from "@visx/legend";

import { Skill } from "src/pages/home/skills/occurences";

export type KeysProps = {
    palette: string[];
    dataset: Skill[];
};

import "./keys.css";

type KeyListItemProps = {
    value: string;
    text: string;
    index: number;
};

function KeyListItem({ value, text, index }: KeyListItemProps) {
    return (
        <LegendItem className="graph-keys__key">
            <svg className="graph-keys-key__image">
                <rect fill={value} />
            </svg>
            <LegendLabel className="graph-keys-key__text">{text}</LegendLabel>
        </LegendItem>
    );
}

type KeyListProps = {
    labels: KeyListItemProps[];
};

function KeyList({ labels }: KeyListProps) {
    return (
        <div className="graph-keys">
            {labels.map((label) => (
                <KeyListItem {...label} key={"graph-key-" + label.index} />
            ))}
        </div>
    );
}

export function Keys({ dataset, palette }: KeysProps) {
    const keys = [dataset.map(({ name }) => name), palette];

    const scale = scaleOrdinal({
        domain: keys[0],
        range: keys[1],
    });

    return (
        <LegendOrdinal scale={scale}>
            {(labels) =>
                labels && <KeyList labels={labels as KeyListItemProps[]} />
            }
        </LegendOrdinal>
    );
}
