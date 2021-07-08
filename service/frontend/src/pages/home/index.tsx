import React from "react";

import { useState, useEffect } from "react";

import { fetcher } from "src/fetcher";

import { Introduction } from "src/pages/home/introduction";
import { Skills } from "src/pages/home/skills";
import { Salaries } from "src/pages/home/salaries";
import { Status } from "src/pages/home/status";
import { Contacts } from "src/pages/home/contacts";

import "./index.css";

export function Home() {
    return (
        <>
            <Introduction />
            <Skills />
            <Salaries />
            <Status />
            <Contacts />
        </>
    );
}
