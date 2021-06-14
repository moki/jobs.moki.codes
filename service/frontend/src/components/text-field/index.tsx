import React, {
    HTMLProps,
    useState,
    ChangeEvent,
    ChangeEventHandler,
    FocusEvent,
    FocusEventHandler,
    KeyboardEvent,
    KeyboardEventHandler,
    MouseEvent,
} from "react";

import { HasTarget, TargetHasDataset } from "src/components/component";

import "./index.css";

export let ID = {
    id: 0,
    s: "text-field",
    next() {
        return this.s + "-" + this.id++;
    },
};

type TextFieldInputProps = {
    value: string;
    handleChange: ChangeEventHandler;
} & HTMLProps<HTMLInputElement>;

export function TextFieldInput({
    id,
    value,
    handleChange,
    placeholder,
}: TextFieldInputProps) {
    const classes = "text-field__input";

    return (
        <input
            type="text"
            value={value}
            className={classes}
            id={id}
            onChange={handleChange}
            placeholder={placeholder}
        />
    );
}

type TextFieldLabelProps = {
    text: string;
} & HTMLProps<HTMLLabelElement>;

export function TextFieldLabel({ id, text }: TextFieldLabelProps) {
    const classes = "text-field__label";

    return (
        <label htmlFor={id} className={classes}>
            {text}
        </label>
    );
}

export type TextFieldProps = {
    label: string;
} & HTMLProps<HTMLDivElement>;

export function TextField({ label, placeholder }: TextFieldProps) {
    const classes = "text-field";
    const id = ID.next();

    const [text, setText] = useState("");

    const handleChange = (e: ChangeEvent & HasTarget) => {
        setText(e.target.value);
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
            {text}
        </div>
    );
}
