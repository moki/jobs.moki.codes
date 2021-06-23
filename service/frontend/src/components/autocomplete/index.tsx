import React, {
    useState,
    useRef,
    HTMLProps,
    FocusEvent,
    KeyboardEvent,
    ChangeEvent,
    MouseEvent,
} from "react";

import { useOutsideEventAwareness } from "src/hooks/use-outside-event-awareness";

import { HasTarget, TargetHasDataset } from "src/components/component";

import { TextField } from "src/components/text-field";

import "./index.css";

export type LookupEntry = { key: string; value: any };

export type LookupResult = LookupEntry[];

type SuggestionsProps = {
    suggestions: LookupResult;
    handlePick: (_: LookupEntry) => void;
    clearUI: () => void;
} & HTMLProps<HTMLElement>;

function Suggestions({ suggestions, handlePick, clearUI }: SuggestionsProps) {
    const [selected, setSelected] = useState("");

    const handleFocus = (
        e: FocusEvent<HTMLLIElement> & HasTarget & TargetHasDataset
    ) => {
        if (!e || !e.target || !e.target.dataset) return;

        const { dataset } = e.target;

        if (
            typeof dataset.key === "undefined" ||
            typeof dataset.value === "undefined"
        )
            return;

        setSelected(e.target.dataset.path);
    };

    const handleKeyPress = (
        e: KeyboardEvent<HTMLLIElement> & HasTarget & TargetHasDataset
    ) => {
        if (!e || !e.target || !e.charCode || !e.target.dataset) return;

        if (e.charCode !== 13 && e.charCode !== 32) return;

        const { dataset } = e.target;

        if (
            typeof dataset.key === "undefined" ||
            typeof dataset.value === "undefined"
        )
            return;

        const { key, value } = dataset;

        handlePick({ key, value });

        clearUI();
    };

    const handleClick = (
        e: MouseEvent<HTMLLIElement> & HasTarget & TargetHasDataset
    ) => {
        if (!e || !e.target || !e.target.dataset) return;

        const { dataset } = e.target;

        if (
            typeof dataset.key === "undefined" ||
            typeof dataset.value === "undefined"
        )
            return;

        const { key, value } = dataset;

        handlePick({ key, value });

        clearUI();
    };

    const classes = {
        list:
            "autocomplete__suggestions" +
            " " +
            "autocomplete-suggestions_" +
            (suggestions.length ? "open" : "close"),
        item: "autocomplete-suggestions__item",
    };

    return (
        <ul className={classes.list}>
            {suggestions.map((pair, i: number) => (
                <li
                    data-key={pair.key}
                    data-value={pair.value}
                    className={classes.item}
                    key={i}
                    tabIndex={0}
                    onFocus={handleFocus}
                    onKeyPress={handleKeyPress}
                    onMouseDown={handleClick}
                >
                    {pair.key}
                </li>
            ))}
        </ul>
    );
}

export interface HasLookup {
    lookup(_: string): LookupResult;
}

export type AutocompleteProps<T> = {
    trie: T;
    label: string;
    handlePick: (_: LookupEntry) => void;
    placeholder: string;
};

export function Autocomplete<T extends HasLookup>({
    trie,
    label,
    handlePick,
    placeholder,
}: AutocompleteProps<T>) {
    const classes = "autocomplete";

    const [text, setText] = useState("");

    const [suggestions, setSuggestions] = useState<LookupResult>([]);

    const changeHandler = (e: ChangeEvent & HasTarget) => {
        if (!e || !e.target) return;

        setText(e.target.value);

        if (e.target.value) {
            setSuggestions(
                trie.lookup(e.target.value.toLowerCase()).map((pair) => pair)
            );
        } else {
            setSuggestions([]);
        }
    };

    const pickHandler = (entry: LookupEntry) => {
        setText(entry.key);

        handlePick(entry);
    };

    const clearUI = () => {
        setText("");

        setSuggestions([]);
    };

    const eventOutsideHandler = () => {
        setText("");

        setSuggestions([]);
    };

    const ref = useRef(null);

    useOutsideEventAwareness(
        ref,
        ["mousedown", "touchstart"],
        eventOutsideHandler
    );

    return (
        <div className={classes} ref={ref}>
            <TextField
                label={label}
                placeholder={placeholder}
                handleChange={changeHandler}
                value={text}
            />
            <Suggestions
                suggestions={suggestions}
                handlePick={pickHandler}
                clearUI={clearUI}
            />
        </div>
    );
}
