export type TagProp = {
    tag: keyof HTMLElementTagNameMap;
};

export type HasTarget = {
    target: HTMLInputElement;
};

export type TargetHasDataset = {
    target: { dataset: { [key: string]: string } };
};

export type TargetHasValue = {
    target: { value: string };
};
