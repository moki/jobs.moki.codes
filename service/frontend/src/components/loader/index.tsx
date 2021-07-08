import React from "react";

import "./index.css";

export function Loader() {
    return (
        <div className="loader">
            <div className="loader__square loader__square-1"></div>
            <div className="loader__square loader__square-2"></div>
            <div className="loader__square loader__square-3"></div>
            <div className="loader__square loader__square-4"></div>
        </div>
    );
}
