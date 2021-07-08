import React, { useEffect, useMemo, HTMLProps } from "react";

import { fetcher } from "src/fetcher";

import { palette } from "src/palette";

import { Section } from "src/components/section";
import { Container } from "src/components/container";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";
import { Text } from "src/components/text";
import { ParentSize } from "@visx/responsive";

import { Loader } from "src/components/loader";

import { DistributionGraph } from "src/pages/home/salaries/distribution-graph";

import "./distribution.css";

export type SalaryQuartiles = {
    lower_fence: number;
    first: number;
    median: number;
    third: number;
    upper_fence: number;
};

export type SalaryCount = [number, number];

export type Skill = {
    name: string;
    quartiles: SalaryQuartiles;
    salary_count: SalaryCount[];
    outliers_low: number[];
    outliers_up: number[];
    sample_size: number;
};

export type State = Skill[];

const endpoint = "api/salaries/quartiles-and-counts";

const selector = (o: any): Skill[] => o;

const delay = 10000;

export function Distribution() {
    const classes = "salaries-distribution";

    return (
        <div className={classes}>
            <DistributionHeader />
            <DistributionContainer />
        </div>
    );
}

function DistributionHeader() {
    return (
        <>
            <Heading tag="h4" level={3}>
                Distribution
            </Heading>
            <SubHeading tag="p" level={3}>
                for skills
            </SubHeading>
            <Text.Component tag="p">
                This graph depicts salaries distribution.
            </Text.Component>
            <Text.Component tag="p">
                If you don't see skill you are interested in — Don't Worry!
            </Text.Component>
            <Text.Component tag="p">
                It's only matter of time for enought datapoints to be
                aggregated, successively calculating distibution.
            </Text.Component>
        </>
    );
}

export function DistributionContainer() {
    const initial: State = [];

    const [setUrl, data, error, loading, restart] = fetcher(
        endpoint,
        "GET",
        initial,
        selector
    );

    useEffect(() => {
        if (error) restart(delay);
    }, [error]);

    const colors = useMemo(
        () =>
            palette(data.length).map(
                (color) =>
                    `hsla(${color[0]}, ${color[1]}%, ${color[2]}%, ${color[3]}%)`
            ),
        [data]
    );

    if (loading || error)
        return (
            <div className="salaries-distribution__container">
                <Loader />
            </div>
        );

    return (
        <div className="salaries-distribution__container">
            <ParentSize className="salaries-distribution-container__responsive">
                {({ width, height }) => (
                    <DistributionGraph
                        height={height}
                        width={width}
                        palette={colors}
                        dataset={data}
                    />
                )}
            </ParentSize>
        </div>
    );
}
