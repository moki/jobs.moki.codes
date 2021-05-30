import { ComponentChildren } from "preact";

export interface HTMLAttributes {
    [key: string]: any;
}

export type TagProp = {
    tag: keyof HTMLElementTagNameMap;
};

export type ChildrenProp = {
    children?: ComponentChildren;
};
