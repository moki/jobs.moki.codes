import React, { useState, useEffect, useMemo } from "react";

import { fetcher } from "src/fetcher";

import { Trie } from "src/trie";

import { palette } from "src/palette";

import { Section } from "src/components/section";
import { Container } from "src/components/container";
import { Heading } from "src/components/heading";
import { SubHeading } from "src/components/sub-heading";
import { Text } from "src/components/text";
import { Label } from "src/components/label";
import { Select, Option } from "src/components/select";
import { Button } from "src/components/button";
import { Autocomplete } from "src/components/autocomplete";
import { Loader } from "src/components/loader";

import { ParentSize } from "@visx/responsive";

import { DominanceGraph } from "src/pages/home/skills/dominance-graph";
import { Keys } from "src/pages/home/skills/keys";

import { CustomWindow } from "src/index";

import "./dominance.css";

export function Dominance() {
    const classes = "skills-dominance";

    return (
        <div className={classes}>
            <DominanceHeader />
            <DominanceContainer />
        </div>
    );
}

export function DominanceHeader() {
    return (
        <>
            <Heading tag="h4" level={3}>
                Dominance
            </Heading>
            <SubHeading tag="p" level={3}>
                over others
            </SubHeading>
            <Text.Component tag="p">
                This graph depicts skill popularity.
            </Text.Component>
        </>
    );
}

export type Skill = {
    name: string;
    dates: number[];
    dominance: number[];
    total_occurences: number;
};

export type State = Skill[];

const endpoint = "api/skills/dominance";
//const endpoint = "https://jobs.moki.codes/api/skills/dominance";

const selector = (o: any): Skill[] => o;

function DominanceContainer() {
    const initial: State = [];

    const [setUrl, data, error, loading, restart] = fetcher(
        endpoint,
        "GET",
        initial,
        selector
    );

    const [dataset, setDataset] = useState<Skill[]>([]);

    useEffect(() => {
        if (error) restart(10000);
    }, [error]);

    const colors = useMemo(
        () =>
            palette(dataset.length).map(
                (color) =>
                    `hsla(${color[0]}, ${color[1]}%, ${color[2]}%, ${color[3]}%)`
            ),
        [dataset]
    );

    if (loading || error)
        return (
            <div className="skills-dominance__graph-container">
                <Loader />
            </div>
        );

    return (
        <>
            <Controls data={data} dataset={dataset} update={setDataset} />
            <div className="skills-dominance__graph-container">
                <ParentSize className="skills-dominance-graph-container__responsive">
                    {({ width, height }) => (
                        <DominanceGraph
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

export type ControlsProps = {
    data: Skill[];
    dataset: Skill[];
    update: (dataset: Skill[]) => void;
};

export function Controls({ data, dataset, update }: ControlsProps) {
    const trie = useMemo(() => {
        return data.reduce(
            (trie, datum, i: number) => trie.insert(datum.name, i),
            new Trie<number>()
        );
    }, []);

    const debugFilters = [
        { k: "top 100", v: 100 },
        {
            k: "all",
            v: data.length,
        },
    ];

    const filters = useMemo(
        () => [
            { k: "top 5", v: 5 },
            { k: "top 10", v: 10 },
            { k: "top 25", v: 25 },
            { k: "top 50", v: 50 },
            ...((window as CustomWindow).debug ? debugFilters : []),
            { k: "none", v: 0 },
        ],
        []
    );

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
        <div className="skills-dominance__controls">
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
