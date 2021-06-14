import React, {
    useState,
    HTMLProps,
    FocusEvent,
    KeyboardEvent,
    ChangeEvent,
    MouseEvent,
} from "react";

import { HasTarget, TargetHasDataset } from "src/components/component";

import { TextFieldLabel, TextFieldInput, ID } from "src/components/text-field";

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

        clearUI();

        handlePick({ key, value });
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

        clearUI();

        handlePick({ key, value });
    };

    const classes = {
        list: "autocomplete__suggestions",
        item: "autocomplete__suggestions-item",
    };

    const styles = {
        display: suggestions.length ? "block" : "none",
    };

    return (
        <ul className={classes.list} style={styles}>
            {suggestions.map((pair, i: number) => (
                <li
                    data-key={pair.key}
                    data-value={pair.value}
                    className={classes.item}
                    key={i}
                    tabIndex={0}
                    onFocus={handleFocus}
                    onKeyPress={handleKeyPress}
                    onClick={handleClick}
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

    const id = ID.next();

    const [text, setText] = useState("");

    const [suggestions, setSuggestions] = useState<LookupResult>([]);

    const handleChange = (e: ChangeEvent & HasTarget) => {
        if (!e || !e.target) return;

        setText(e.target.value);

        if (e.target.value) {
            setSuggestions(trie.lookup(e.target.value).map((pair) => pair));
        } else {
            setSuggestions([]);
        }
    };

    const clearUI = () => {
        setText("");

        setSuggestions([]);
    };

    return (
        <div className={classes}>
            <TextFieldLabel id={id} text={label} />
            <TextFieldInput
                id={id}
                value={text}
                handleChange={handleChange}
                placeholder={placeholder}
            />
            <Suggestions
                suggestions={suggestions}
                handlePick={handlePick}
                clearUI={clearUI}
            />
        </div>
    );
}
