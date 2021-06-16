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
import { Select } from "src/components/select";
import { Button } from "src/components/button";
import { Autocomplete } from "src/components/autocomplete";

import { ParentSize, ScaleSVG } from "@visx/responsive";

import { AxisLeft, AxisBottom } from "@visx/axis";
import { GridRows, GridColumns } from "@visx/grid";

import { scaleLinear, scaleTime, scaleOrdinal } from "@visx/scale";
import { extent, max } from "d3-array";

import { LinePath } from "@visx/shape";
import { curveStepAfter, curveLinear } from "@visx/curve";
import { LegendOrdinal, LegendItem, LegendLabel } from "@visx/legend";

import "./occurences.css";

type skill = {
    name: string;
    dates: number[];
    occurences: number[];
    total_occurences: number;
};

type state = skill[];

type KeysProps = {
    keys: [string, [number, number, number, number]][];
};

// The whole thing is pretty ugly
// TODO: rewrite

function Keys({ keys }: KeysProps) {
    const scale = scaleOrdinal({
        domain: keys.map(([name]) => name),
        range: keys.map(([_, color], i: number) => {
            return `hsla(${color[0]}, ${color[1]}%, ${color[2]}%, ${color[3]}%`;
        }),
    });

    return (
        <LegendOrdinal scale={scale}>
            {(labels) => (
                <div
                    style={{
                        display: "flex",
                        flexDirection: "row",
                        flexWrap: "wrap",
                        //gap: "var(--gc)",
                        paddingTop: "calc(var(--gc) * 4)",
                    }}
                >
                    {labels.map((label, i) => (
                        <LegendItem key={`legend-quantile-${i}`} margin="0 0">
                            <svg width={"12"} height={"12"}>
                                <rect
                                    fill={label.value}
                                    width={"12"}
                                    height={"12"}
                                />
                            </svg>
                            <LegendLabel
                                align="left"
                                margin="calc(var(--gc)/2) var(--gc)"
                            >
                                {label.text}
                            </LegendLabel>
                        </LegendItem>
                    ))}
                </div>
            )}
        </LegendOrdinal>
    );
}

type GraphProps = {
    dataset: state;
    width: number;
    height: number;
} & HTMLProps<HTMLDivElement>;

function Graph({ dataset, width, height }: GraphProps) {
    const colors = palette(dataset.length);

    let ays = [];

    const dl = dataset.length;

    const max_occ_len = dataset.length ? dataset[0].occurences.length : 0;

    let min_xs_i = 0;

    for (let i = 0; i < dl; i++) {
        ays.push(...dataset[i].occurences);

        if (dataset[i].dates.length < min_xs_i)
            min_xs_i = dataset[i].dates.length;
    }

    const axs = dataset.length
        ? dataset[min_xs_i].dates.map((x: number, i: number) => {
              const day = x % 100;

              x = Math.floor(x / 100);

              const month = x % 100;

              x = Math.floor(x / 100);

              return new Date(Date.UTC(x, month - 1, day));
          })
        : [];

    const [ymin, ymax] = extent(ays) as [number, number];

    const yScale = scaleLinear({
        domain: [ymin, ymax],
        range: [height, 0],
    });

    const [xmin, xmax] = extent(axs) as [Date, Date];
    const xScale = scaleTime({
        domain: [xmin, xmax],
        range: [0, width],
    });

    const bticks =
        width + 32 * 2 + 16 * 2 < 720 ? max_occ_len / 4 : max_occ_len / 2;

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
            <AxisBottom top={height} scale={xScale} numTicks={bticks} />
            <AxisLeft left={0} scale={yScale} />
            {dataset.map((data, i: number) => {
                const xs = axs;

                const ys = data.occurences;

                if (xs.length !== ys.length) return null;

                const _d = zip(xs, ys);

                const stroke = `hsla(${colors[i][0]}, ${colors[i][1]}%, ${colors[i][2]}%, ${colors[i][3]}%`;

                return (
                    <LinePath
                        curve={curveLinear}
                        data={_d}
                        x={(d) => xScale(d[0] ?? xmin)}
                        y={(d) => yScale(d[1] ?? ymin)}
                        stroke={stroke}
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

const endpoint = "api/skills";

const selector = (o: any): skill[] => o;

export function Occurences(this: any) {
    const classes = "occurences";

    const initial: state = [];
    const trie = new Trie<number>();

    const [setUrl, data, error, loading, restart] = fetcher(
        endpoint,
        "GET",
        initial,
        selector
    );

    const [dataset, setDataset] = useState<skill[]>([]);

    const [top, setTop] = useState<number>(5);

    const handleSelect = (top: number) => {
        setTop(top);
    };

    const handleAddSkill = ({ key, value }: { key: string; value: number }) => {
        const n = [...dataset];

        if (!n.includes(data[value])) n.push(data[value]);

        setDataset(n);
    };

    const handleReset = () => {
        setTop(0);
        setDataset([]);
    };

    if (!loading && !error) {
        for (let i = 0; i < data.length; i++) {
            trie.insert(data[i].name, i);
        }
    }

    useEffect(() => {
        setDataset(data.slice(0, top));
    }, [loading, error, top]);

    useEffect(() => {
        if (error) restart(10000);
    }, [error]);

    let keys;
    if (!loading && !error) {
        const colors = palette(data.length);

        keys = zip(
            dataset.map(({ name }) => name),
            colors!
        );
    }

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
            {!loading && !error && keys && (
                <>
                    <div style={{ display: "flex" }}>
                        <Select
                            active={top.toString()}
                            handleSelect={handleSelect}
                            label="filter"
                            options={[
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
                            ]}
                        />
                        <div style={{ paddingLeft: "calc(var(--gc) * 2)" }}>
                            <Label text="prestine" id="button-reset" />
                            <Button
                                type="outlined"
                                onClick={handleReset}
                                id="button-reset"
                            >
                                reset
                            </Button>
                        </div>
                    </div>
                    <Autocomplete
                        trie={trie}
                        label="add skill"
                        placeholder="try typing react or smth"
                        handlePick={handleAddSkill}
                    />
                    <div className="occurences__graph-container">
                        <ParentSize>
                            {({ width, height }) => (
                                <Graph
                                    height={height}
                                    width={width}
                                    dataset={dataset}
                                />
                            )}
                        </ParentSize>
                    </div>
                    <Keys keys={keys} />
                </>
            )}
        </div>
    );
}
