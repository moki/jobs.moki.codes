import React, { ChangeEvent } from "react";

import { Label } from "src/components/label";

import { HasTarget, TargetHasValue } from "src/components/component";

import "./index.css";

export let ID = {
    s: "select",
    id: 0,
    next() {
        return this.s + "-" + this.id++;
    },
};

export type HandleSelect = (_: any) => void;

export type Option = {
    k: string;
    v: any;
};

export type OptionProps = Option;

export function Option({ k, v }: OptionProps) {
    const classes = "select__item";

    return (
        <option className={classes} value={v}>
            {k}
        </option>
    );
}

export type SelectProps = {
    active: any;
    options: Option[];
    label: string;
    handleSelect: HandleSelect;
};

export function Select({ label, options, handleSelect, active }: SelectProps) {
    const classes = {
        base: "select",
        select: "select__selector",
        label: "select__label",
    };

    const id = ID.next();

    const handleChange = (
        e: ChangeEvent<HTMLSelectElement> & HasTarget & TargetHasValue
    ) => {
        handleSelect(e.target.value);
    };

    return (
        <div className={classes.base}>
            <Label id={id} text={label} />
            <select
                className={classes.select}
                id={id}
                onChange={handleChange}
                value={active}
            >
                {options.map((o, i: number) => (
                    <Option {...o} key={i} />
                ))}
            </select>
        </div>
    );
}
