import React, {
    HTMLProps,
    useState,
    useRef,
    useEffect,
    MouseEvent,
    KeyboardEvent,
} from "react";

import { Label } from "src/components/label";

import { useOutsideEventAwareness } from "src/hooks/use-outside-event-awareness";

import "./index.css";

const ID = {
    s: "select",
    id: 0,
    next() {
        return this.s + "-" + this.id++;
    },
};

export type Option = {
    k: string;
    v: any;
};

type OptionProps = Option & {
    handleClick: (_: { k: string; v: any }) => void;
};

function Option({ k, v, handleClick }: OptionProps) {
    const classes = "select-options-suggestions__suggestion";

    const clickHandler = (e: MouseEvent) => handleClick({ k, v });

    const keyUpHandler = (e: KeyboardEvent) =>
        e.key === "Enter" && handleClick({ k, v });

    return (
        <div
            className={classes}
            onMouseUp={clickHandler}
            onKeyUp={keyUpHandler}
            tabIndex={0}
        >
            {k}
        </div>
    );
}

export type HandleSelect = (_: any) => void;

type OptionsProps = {
    options: Option[];
    active: Option;
    id: string;
    handleSelect: HandleSelect;
};

function Options({ options, active, id, handleSelect }: OptionsProps) {
    const [open, setOpen] = useState(false);

    const [selected, setSelected] = useState(active);

    const eventOutsideHandler = () => setOpen(false);

    const ref = useRef(null);

    useOutsideEventAwareness(
        ref,
        ["mousedown", "touchstart"],
        eventOutsideHandler
    );

    useEffect(() => setSelected(active), [active]);

    const classes = {
        options: "select__options",
        selected: "select-options__selected",
        suggestions:
            "select-options__suggestions select-options__suggestions_" +
            (open ? "open" : "close"),
    };

    const clickHandler = (pair: Option) => {
        setSelected(pair);
        setOpen(false);
        handleSelect && handleSelect(pair);
    };

    return (
        <div className={classes.options} ref={ref}>
            <div
                className={classes.selected}
                onClick={() => setOpen(!open)}
                onKeyUp={(e: KeyboardEvent) =>
                    e.key === "Enter" && setOpen(!open)
                }
                id={id}
                tabIndex={0}
            >
                {selected.k}
            </div>
            <div className={classes.suggestions}>
                {options
                    .filter(({ k }) => k !== selected.k)
                    .map((o, i: number) => (
                        <Option
                            {...o}
                            handleClick={clickHandler}
                            key={id + "-" + i}
                        />
                    ))}
            </div>
        </div>
    );
}

export type SelectProps = {
    options: Option[];
    selected: Option;
    label?: string;
    handleSelect: HandleSelect;
};

export function Select({
    options,
    selected,
    label,
    handleSelect,
}: SelectProps) {
    const id = ID.next();

    return (
        <div className="select">
            {label && <Label text={label} id={id} />}
            {options && (
                <Options
                    options={options}
                    handleSelect={handleSelect}
                    active={selected}
                    id={id}
                />
            )}
        </div>
    );
}
