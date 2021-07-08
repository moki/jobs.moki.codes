import React, {
    HTMLProps,
    memo,
    useMemo,
    useCallback,
    PointerEvent,
} from "react";

import { scaleLinear, scaleBand } from "@visx/scale";

import { ScaleSVG } from "@visx/responsive";
import { Group } from "@visx/group";

import { GridRows, GridColumns } from "@visx/grid";
import { AxisBottom } from "@visx/axis";
import { Text } from "@visx/text";
import { ViolinPlot, BoxPlot } from "@visx/stats";
import { PatternLines } from "@visx/pattern";
import { useTooltip, useTooltipInPortal } from "@visx/tooltip";

import { Text as Body } from "src/components/text";
import { SubHeading } from "src/components/sub-heading";

import { State, SalaryQuartiles } from "src/pages/home/salaries/distribution";

import "./distribution-graph.css";

export type DistributionGraphProps = {
    height: number;
    width: number;
    palette: string[];
    dataset: State;
} & HTMLProps<HTMLDivElement>;

function DistributionGraphComponent({
    dataset,
    palette,
    height,
    width,
}: DistributionGraphProps) {
    dataset = useMemo(
        () => dataset.sort((a, b) => b.quartiles.median - a.quartiles.median),
        [dataset]
    );

    const xs = useMemo(
        () =>
            dataset.reduce(
                (acc, datum) => [
                    ...acc,
                    0,
                    datum.quartiles.first,
                    datum.quartiles.median,
                    datum.quartiles.third,
                    datum.quartiles.upper_fence,
                    ...datum.salary_count.map(([value, _]) => value),
                ],
                [] as number[]
            ),
        [dataset]
    ).sort((a, b) => a - b);

    const glyphWidth = useMemo(
        () => (width > 600 ? ((9 * width) / 16 / dataset.length) * 10 : 80),
        [width, dataset]
    );

    const glyphPadding = 2;

    const xsMax = useMemo(() => Math.max(...xs), [dataset]);
    const xsMin = useMemo(() => Math.min(...xs), [dataset]);

    const xScale = useMemo(
        () =>
            scaleLinear<number>({
                range: [0, width],
                round: true,
                domain: [xsMin, xsMax],
            }),
        [width, dataset]
    );

    const xFormat = useCallback(
        xScale.tickFormat(xScale.range().length, "($~s"),
        [xScale]
    );

    const height_ = useMemo(
        () => dataset.length * glyphWidth + (dataset.length - 1) * glyphPadding,
        [dataset, width]
    );

    const yScale = useMemo(
        () =>
            scaleBand<string>({
                range: [0, height_],
                round: true,
                domain: dataset.map((datum) => datum.name),
                padding: 0.1,
            }),
        [height_, dataset]
    );

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

    const { containerRef, TooltipInPortal } = useTooltipInPortal({
        detectBounds: true,
        scroll: true,
        debounce: 5,
    });

    const tooltipClasses =
        "theme theme_typography theme_color_light dominance-graph__tooltip";

    return (
        <>
            <ScaleSVG height={height_} width={width}>
                <GridColumns
                    scale={xScale}
                    height={height_}
                    stroke="var(--color-on-surface)"
                    strokeOpacity={0.25}
                />
                <AxisBottom top={0 - 16} scale={xScale} tickFormat={xFormat} />
                <AxisBottom top={height_} scale={xScale} tickFormat={xFormat} />
                {dataset.map((datum, i: number) => (
                    <Distribution
                        key={`distribution-${datum.name}`}
                        name={datum.name}
                        data={datum.salary_count}
                        quartiles={datum.quartiles}
                        underpay={datum.outliers_low}
                        overpay={datum.outliers_up}
                        left={yScale(datum.name)!}
                        width={yScale.bandwidth()}
                        top={i * (glyphPadding + glyphWidth)}
                        color={palette[i]}
                        xScale={xScale}
                        yScale={yScale}
                        showTooltip={showTooltip}
                        hideTooltip={hideTooltip}
                        index={i}
                    />
                ))}
            </ScaleSVG>
            {tooltipOpen && (
                <TooltipInPortal
                    key={ID.next()}
                    top={tooltipTop}
                    left={tooltipLeft}
                    className={tooltipClasses}
                    unstyled={true}
                >
                    <TooltipContent {...tooltipData} />
                </TooltipInPortal>
            )}
        </>
    );
}

type DistributionProps = {
    data: [number, number][];
    name: string;
    quartiles: SalaryQuartiles;
    underpay: number[];
    overpay: number[];
    xScale: any;
    yScale: any;
    showTooltip: any;
    hideTooltip: any;
    index: number;
    left: number;
    top: number;
    width: number;
    color: string;
};

function Distribution({
    left,
    top,
    width,
    color,
    index,
    data,
    name,
    quartiles,
    underpay,
    overpay,
    xScale,
    yScale,
    showTooltip,
    hideTooltip,
}: DistributionProps) {
    const patternId = `distribution-pattern-${index}`;

    const moveHandler = (e: PointerEvent<SVGElement>) => {
        if (
            !event ||
            !event.target ||
            !(event.target instanceof SVGElement) ||
            typeof event.target.getBoundingClientRect === "undefined"
        )
            return;

        const rectangle = (e.target as SVGElement).getBoundingClientRect();

        showTooltip({
            tooltipTop: e.clientY,
            tooltipLeft: e.clientX,
            tooltipData: { name, quartiles, underpay, overpay },
        });
    };

    const handleLeave = () => {
        hideTooltip();
    };

    return (
        <Group
            top={top}
            onPointerMove={moveHandler}
            onPointerLeave={handleLeave}
        >
            <PatternLines
                id={patternId}
                height={4}
                width={4}
                stroke={color}
                strokeWidth={0.25}
                orientation={["vertical"]}
            />
            <ViolinPlot
                left={left}
                width={width}
                data={data}
                count={([_, count]) => count}
                value={([value, _]) => value}
                valueScale={xScale}
                stroke={color}
                opacity={0.75}
                fill={`url(#${patternId})`}
                horizontal={true}
            />
            <BoxPlot
                max={0}
                min={quartiles.upper_fence}
                firstQuartile={quartiles.first}
                thirdQuartile={quartiles.third}
                median={quartiles.median}
                top={width / 2.5}
                boxWidth={width / 5}
                fill={color}
                fillOpacity={0.65}
                stroke="var(--color-on-background)"
                strokeWidth={1}
                valueScale={xScale}
                outliers={[...underpay, ...overpay]}
                horizontal={true}
            />
            <Text
                dy={width * 0.25 + 4}
                dx={16}
                fill={color}
                className="salaries-distribution-graph-distribution__label"
            >
                {name}
            </Text>
        </Group>
    );
}

const ID = {
    s: "tooltip-salaries-distribution-",
    ids: new Array(10).fill(0).map((x) => Math.random()),
    i: 0,
    next() {
        const next = this.s + this.ids[this.i];

        this.i = (this.i + 1) % 10;

        return next;
    },
};

type TooltipData = {
    quartiles: SalaryQuartiles;
    name: string;
    underpay?: number[];
    overpay?: number[];
};

function TooltipContent({ name, quartiles, underpay, overpay }: TooltipData) {
    return (
        <>
            <SubHeading tag="div" level={3}>
                {name && name}
            </SubHeading>
            {underpay && underpay.length && (
                <Quartile data={underpay} name="underpay" />
            )}
            <Quartile data={Math.round(quartiles.first)} name="low" />
            <Quartile data={Math.round(quartiles.median)} name="median" />
            <Quartile data={Math.round(quartiles.third)} name="high" />
            <Quartile
                data={Math.round(quartiles.upper_fence)}
                name="upper fence"
            />

            {overpay && overpay.length ? (
                <Quartile data={overpay} name="overpay" />
            ) : (
                ""
            )}
        </>
    );
}

type QuartileProps = {
    data: number | number[];
    name: string;
};

function Quartile({ name, data }: QuartileProps) {
    return (
        <div className="salaries-distribution-graph-tooltip__item">
            <SubHeading
                tag="span"
                level={3}
                className="salaries-distribution-graph-tooltip-item__heading"
            >
                {name}
            </SubHeading>
            <Body.Component
                tag="span"
                className="salaries-distribution-graph-tooltip-item__body"
            >
                $
                {Array.isArray(data)
                    ? data.length === 1
                        ? data[0]
                        : data[0] + "..." + data[data.length - 1]
                    : data}
            </Body.Component>
        </div>
    );
}

export const DistributionGraph = memo(
    DistributionGraphComponent,
    (previous: DistributionGraphProps, next: DistributionGraphProps) =>
        !next.dataset.length &&
        JSON.stringify(previous.dataset) === JSON.stringify(next.dataset)
);
