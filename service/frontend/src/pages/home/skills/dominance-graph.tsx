import React, {
    useMemo,
    useCallback,
    memo,
    HTMLProps,
    PointerEvent,
} from "react";

import { zip } from "ramda";

import { scaleLinear, scaleTime } from "@visx/scale";
import { extent, max, bisector } from "d3-array";
import { ScaleSVG } from "@visx/responsive";
import { GridRows, GridColumns } from "@visx/grid";
import { AxisLeft, AxisBottom } from "@visx/axis";
import { Group } from "@visx/group";
import { LinePath, Bar, Line } from "@visx/shape";
import { curveLinear } from "@visx/curve";
import { localPoint } from "@visx/event";
import { useTooltip, useTooltipInPortal } from "@visx/tooltip";

import { Text } from "src/components/text";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";

import { State, Skill } from "src/pages/home/skills/dominance";

import "./dominance-graph.css";

const ID = {
    s: "tooltip-",
    ids: new Array(10).fill(0).map((x) => Math.random()),
    i: 0,
    next() {
        const next = this.s + this.ids[this.i];

        this.i = (this.i + 1) % 10;

        return next;
    },
};

type TooltipDatum = {
    name: string;
    dominance: number;
    color: string;
};

type TooltipData = { data: TooltipDatum[]; date: Date };

type TooltipContentEntryProps = TooltipDatum;

function TooltipContentEntry({
    name,
    dominance,
    color,
}: TooltipContentEntryProps) {
    return (
        <div className="dominance-graph-tooltip__item">
            <span className="dominance-graph-tooltip-item__key">
                <span
                    style={{
                        backgroundColor: color,
                    }}
                    className="dominance-graph-tooltip-item__color"
                ></span>
                <SubHeading
                    tag="span"
                    level={3}
                    className="dominance-graph-tooltip-item__heading"
                >
                    {name}
                </SubHeading>
            </span>
            <Text.Component
                tag="span"
                className="dominance-graph-tooltip-item__body"
            >
                {Math.round(dominance * 100) / 100}%
            </Text.Component>
        </div>
    );
}

function TooltipContent({ content }: { content: TooltipData }) {
    if (!content || !content.data.length) return <span />;

    return (
        <>
            <SubHeading tag="div" level={3}>
                {content.date.toLocaleDateString("en-US", {
                    day: "numeric",
                    month: "short",
                    year: "numeric",
                })}
            </SubHeading>
            {content.data.map((datum: TooltipDatum, i: number) => (
                <TooltipContentEntry {...datum} key={"tooltip-row-" + i} />
            ))}
        </>
    );
}

export type DominanceGraphProps = {
    dataset: State;
    width: number;
    height: number;
    palette: string[];
} & HTMLProps<HTMLDivElement>;

export const DominanceGraph = memo(
    function DominanceGraph({
        dataset,
        palette,
        width,
        height,
    }: DominanceGraphProps) {
        if (!dataset.length)
            return (
                <div className="dominance__graph-placeholder">
                    <Text.Component tag="p">add datapoints</Text.Component>
                </div>
            );

        const ys = useMemo(
            () =>
                dataset.reduce((acc, occ) => {
                    return [...acc, ...occ.dominance];
                }, [] as Array<number>),
            [dataset]
        );

        const xs = useMemo(
            () =>
                dataset[0].dates.map((x: number, i: number) => {
                    const day = x % 100;

                    x = Math.floor(x / 100);

                    const month = x % 100;

                    x = Math.floor(x / 100);

                    return new Date(x, month - 1, day);
                }),
            [dataset[0].dates]
        );

        const [ymin, ymax] = useMemo(() => extent(ys), [ys]) as [
            number,
            number
        ];

        const yScale = useMemo(
            () =>
                scaleLinear({
                    domain: [ymin, ymax],
                    range: [height, 0],
                }),
            [ys, ymin, ymax, height]
        );

        const [xmin, xmax] = useMemo(() => extent(xs), [xs]) as [Date, Date];

        const xScale = useMemo(
            () =>
                scaleTime({
                    domain: [xmin, xmax],
                    range: [0, width],
                }),
            [xs, xmin, xmax, width]
        );

        const ticks = useMemo(() => (width - 720 < 0 ? 4 : 15), [width]);

        const {
            showTooltip,
            hideTooltip,
            tooltipOpen,
            tooltipData = {} as TooltipData,
            tooltipLeft,
            tooltipTop,
        } = useTooltip({
            tooltipOpen: false,
            tooltipData: {} as TooltipData,
        });

        const createTooltip = useMemo(() => dataset.length < 101, [dataset]);

        const { containerRef, TooltipInPortal } = useTooltipInPortal({
            detectBounds: createTooltip,
            scroll: createTooltip,
            debounce: 10,
        });

        const bisectDate = useMemo(() => bisector((d: Date) => d).left, []);

        const bisectNumber = useMemo(() => bisector((n: number) => n).left, []);

        const findClosest = useCallback(
            (v: number, vs: any[], scaleFn: any, biFn: any) => {
                const v0 = scaleFn.invert(v);
                const i = biFn(vs, v0, 1);

                const l = vs[i - 1] ?? vs[i];
                const r = vs[i];

                return v0.valueOf() - l.valueOf() > r.valueOf() - v0.valueOf()
                    ? [r, i]
                    : [l, i - 1];
            },
            [xs, width]
        );

        const tooltipClasses =
            "theme theme_typography theme_color_light dominance-graph__tooltip";

        const handlePointerMove = useCallback(
            (event: PointerEvent<SVGElement>) => {
                if (
                    !event ||
                    !event.target ||
                    !(event.target instanceof SVGElement) ||
                    dataset.length > 100
                )
                    return;

                const coords = localPoint(event.target.ownerSVGElement!, event);

                const [x, xi] = findClosest(
                    (coords && coords.x) ?? 0,
                    xs,
                    xScale,
                    bisectDate
                );

                const data = {
                    data: dataset
                        .map((data, i: number) => ({
                            name: data.name,
                            dominance: data.dominance[xi],
                            color: palette[i],
                        }))
                        .sort((a, b) => b.dominance - a.dominance),
                    date: x,
                };

                showTooltip({
                    tooltipData: data,
                    tooltipLeft: xScale(x),
                    tooltipTop: (coords && coords.y) ?? 0,
                });
            },
            [showTooltip, width, dataset]
        );

        const handlePointerLeave = (event: PointerEvent<SVGElement>) => {
            hideTooltip();
        };

        return (
            <>
                <ScaleSVG width={width} height={height} innerRef={containerRef}>
                    <GridRows
                        scale={yScale}
                        width={width}
                        stroke="var(--color-on-surface)"
                        strokeOpacity={0.25}
                    />
                    <GridColumns
                        scale={xScale}
                        height={height}
                        stroke="var(--color-on-surface)"
                        strokeOpacity={0.25}
                    />
                    <AxisBottom top={height} scale={xScale} numTicks={ticks} />
                    <AxisLeft scale={yScale} />
                    {dataset.map((data, i: number) => (
                        <Group key={"graph-group-" + i}>
                            <LinePath
                                curve={curveLinear}
                                data={zip<Date, number>(xs, data.dominance)}
                                x={([x, _]) => xScale(x ?? xmin)}
                                y={([_, y]) => yScale(y ?? ymin)}
                                stroke={palette[i]}
                                strokeWidth={1.75}
                                strokeOpacity={1}
                                key={"graph-line-" + i}
                                shapeRendering="geometricPrecision"
                            />
                            {data.dominance.map((o: number, j: number) => (
                                <circle
                                    r={3}
                                    cx={xScale(xs[j])}
                                    cy={yScale(o)}
                                    stroke={palette[i]}
                                    fill={palette[i]}
                                    key={"graph-point-" + i + "-" + j}
                                />
                            ))}
                        </Group>
                    ))}
                    {createTooltip && (
                        <Bar
                            x={0}
                            y={0}
                            height={height}
                            width={width}
                            fill="transparent"
                            onPointerMove={handlePointerMove}
                            onPointerLeave={handlePointerLeave}
                        />
                    )}
                    {tooltipOpen && (
                        <Line
                            from={{ x: tooltipLeft, y: 0 }}
                            to={{ x: tooltipLeft, y: height }}
                            stroke="var(--color-on-surface)"
                            strokeWidth="calc(var(--gc) / 2)"
                            opacity=".5"
                            pointerEvents="none"
                            strokeDasharray="var(--gc)"
                        />
                    )}
                </ScaleSVG>
                {tooltipOpen && createTooltip && (
                    <TooltipInPortal
                        key={ID.next()}
                        top={tooltipTop}
                        left={tooltipLeft}
                        className={tooltipClasses}
                        unstyled={true}
                    >
                        <TooltipContent content={tooltipData} />
                    </TooltipInPortal>
                )}
            </>
        );
    },
    (previous: DominanceGraphProps, next: DominanceGraphProps) =>
        !previous.dataset.length &&
        JSON.stringify(previous.dataset) === JSON.stringify(next.dataset)
);
