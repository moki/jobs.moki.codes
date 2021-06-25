import React, { useState, useEffect, HTMLProps } from "react";

import { fetcher } from "src/fetcher";
import { zip } from "ramda";

import { Trie } from "src/trie";

import { palette } from "src/palette";

import { Section } from "src/components/section";
import { Container } from "src/components/container";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";
import { Text } from "src/components/text";

import { TextField } from "src/components/text-field";
import { Label } from "src/components/label";
import { Select, Option } from "src/components/select";
import { Button } from "src/components/button";
import { Autocomplete } from "src/components/autocomplete";

import { ParentSize, ScaleSVG } from "@visx/responsive";

import { AxisLeft, AxisBottom } from "@visx/axis";
import { GridRows, GridColumns } from "@visx/grid";

import { scaleLinear, scaleTime, scaleOrdinal } from "@visx/scale";
import { extent, max } from "d3-array";

import { LinePath } from "@visx/shape";
import { curveLinear } from "@visx/curve";
import { LegendOrdinal, LegendItem, LegendLabel } from "@visx/legend";

import "./occurences.css";

type skill = {
    name: string;
    dates: number[];
    occurences: number[];
    total_occurences: number;
};

type state = skill[];

export type ControlsProps = {
    data: skill[];
    dataset: skill[];
    update: (dataset: skill[]) => void;
};

export function Controls({ data, dataset, update }: ControlsProps) {
    let trie = new Trie<number>();

    for (let i = 0; i < data.length; i++) {
        trie.insert(data[i].name, i);
    }

    const filters = [
        { k: "top 5", v: 5 },
        { k: "top 10", v: 10 },
        { k: "top 25", v: 25 },
        { k: "top 50", v: 50 },
        { k: "top 100", v: 100 },
        {
            k: "all",
            v: data.length,
        },
        { k: "none", v: 0 },
    ];

    const [top, setTop] = useState<Option>(filters[0]);

    const selectHandler = (option: Option) => {
        setTop(option);
    };

    const handleAddSkill = ({ key, value }: { key: string; value: number }) => {
        if (!dataset.includes(data[value])) update([...dataset, data[value]]);
    };

    const handleReset = () => {
        setTop(filters[filters.length - 1]);

        update([]);
    };

    useEffect(() => update(data.slice(0, top.v)), [top]);

    return (
        <div className="occurences__controls">
            <Select
                label="filter"
                options={filters}
                selected={top}
                handleSelect={selectHandler}
            />
            <div>
                <Label text="prestine" id="button-reset" />
                <Button type="outlined" onClick={handleReset} id="button-reset">
                    reset
                </Button>
            </div>
            <Autocomplete
                trie={trie}
                label="add skill"
                placeholder="try typing react or smth"
                handlePick={handleAddSkill}
            />
        </div>
    );
}

type GraphProps = {
    dataset: state;
    width: number;
    height: number;
    palette: string[];
} & HTMLProps<HTMLDivElement>;

function Graph({ dataset, palette, width, height }: GraphProps) {
    if (!dataset.length)
        return (
            <div className="occurences__graph-placeholder">
                <Text.Component tag="p">add datapoints</Text.Component>
            </div>
        );

    const ys = dataset.reduce((acc, occ) => {
        return [...acc, ...occ.occurences];
    }, [] as Array<number>);

    const xs = dataset[0].dates.map((x: number, i: number) => {
        const day = x % 100;

        x = Math.floor(x / 100);

        const month = x % 100;

        x = Math.floor(x / 100);

        return new Date(Date.UTC(x, month - 1, day));
    });

    const [ymin, ymax] = extent(ys) as [number, number];
    const yScale = scaleLinear({
        domain: [ymin, ymax],
        range: [height, 0],
    });

    const [xmin, xmax] = extent(xs) as [Date, Date];
    const xScale = scaleTime({
        domain: [xmin, xmax],
        range: [0, width],
    });

    const ticks =
        width + 32 * 2 + 16 * 2 < 720
            ? dataset[0].occurences.length / 4
            : dataset[0].occurences.length / 2;

    return (
        <ScaleSVG width={width} height={height}>
            <GridRows
                scale={yScale}
                width={width}
                height={height}
                stroke="var(--color-on-surface)"
                strokeOpacity={0.25}
            />
            <GridColumns
                scale={xScale}
                width={width}
                height={height}
                stroke="var(--color-on-surface)"
                strokeOpacity={0.25}
            />
            <AxisBottom top={height} scale={xScale} numTicks={ticks} />
            <AxisLeft left={0} scale={yScale} />
            {dataset.map((data, i: number) => {
                const _d = zip(xs, data.occurences);

                return (
                    <LinePath
                        curve={curveLinear}
                        data={_d}
                        x={(d) => xScale(d[0] ?? xmin)}
                        y={(d) => yScale(d[1] ?? ymin)}
                        stroke={palette[i]}
                        strokeWidth={1.75}
                        strokeOpacity={1}
                        key={i}
                        shapeRendering="geometricPrecision"
                    />
                );
            })}
        </ScaleSVG>
    );
}

type KeysProps = {
    palette: string[];
    dataset: skill[];
};

function Keys({ dataset, palette }: KeysProps) {
    const keys = [dataset.map(({ name }) => name), palette];

    const scale = scaleOrdinal({
        domain: keys[0],
        range: keys[1],
    });

    return (
        <LegendOrdinal scale={scale}>
            {(labels) => (
                <div className="graph-keys">
                    {labels.map((label, i) => (
                        <LegendItem
                            key={"graph-key-" + i}
                            className="graph-keys__key"
                        >
                            <svg className="graph-keys-key__image">
                                <rect fill={label.value} />
                            </svg>
                            <LegendLabel className="graph-keys-key__text">
                                {label.text}
                            </LegendLabel>
                        </LegendItem>
                    ))}
                </div>
            )}
        </LegendOrdinal>
    );
}

export type OccurencesGraphProps = {
    dataset: skill[];
};

function OccurencesGraph({ dataset }: OccurencesGraphProps) {
    const colors = palette(dataset.length).map(
        (color) => `hsla(${color[0]}, ${color[1]}%, ${color[2]}%, ${color[3]}%)`
    );

    return (
        <>
            <div className="occurences__graph-container">
                <ParentSize className="occurences_graph-container-responsive">
                    {({ width, height }) => (
                        <Graph
                            height={height}
                            width={width}
                            dataset={dataset}
                            palette={colors}
                        />
                    )}
                </ParentSize>
            </div>
            <Keys dataset={dataset} palette={colors} />
        </>
    );
}

const endpoint = "api/skills";

const selector = (o: any): skill[] => o;

export function Occurences() {
    const classes = "occurences";

    const initial: state = [];

    const [setUrl, data, error, loading, restart] = fetcher(
        endpoint,
        "GET",
        initial,
        selector
    );

    const [dataset, setDataset] = useState<skill[]>([]);

    useEffect(() => {
        if (error) restart(10000);
    }, [error]);

    return (
        <div className={classes}>
            <Heading tag="h4" level={3}>
                Occurences
            </Heading>
            <SubHeading tag="p" level={3}>
                occurence over time
            </SubHeading>
            <Text.Component tag="p">
                This graph depicts skill popularity.
            </Text.Component>
            {!loading && !error && (
                <>
                    <Controls
                        data={data}
                        dataset={dataset}
                        update={setDataset}
                    />
                    <OccurencesGraph dataset={dataset} />
                </>
            )}
        </div>
    );
}
