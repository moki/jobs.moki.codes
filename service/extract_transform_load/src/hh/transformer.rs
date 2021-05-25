use chrono::{DateTime, Utc};
use serde::Deserialize;

#[derive(Deserialize, Debug)]
pub(crate) struct Job {
    pub(crate) id: String,
    #[serde(rename(deserialize = "name"), deserialize_with = "de_title")]
    pub(crate) title: String,
    #[serde(deserialize_with = "de_salary")]
    pub(crate) salary: Option<Salary>,
    #[serde(deserialize_with = "de_area")]
    pub(crate) area: String,
    #[serde(rename(deserialize = "schedule"), deserialize_with = "de_schedule")]
    pub(crate) remote: bool,
    #[serde(
        rename(deserialize = "published_at"),
        deserialize_with = "de_published_at"
    )]
    pub(crate) created: DateTime<Utc>,
    #[serde(deserialize_with = "de_specializations")]
    pub(crate) specializations: Vec<Vec<String>>,
    #[serde(deserialize_with = "de_skills", rename(deserialize = "key_skills"))]
    pub(crate) skills: Vec<String>,
    #[serde(deserialize_with = "de_experience")]
    pub(crate) experience: u8,
}

#[derive(Deserialize, Debug)]
pub(crate) struct Salary {
    pub(crate) avg: u64,
    pub(crate) currency: String,
}

pub(crate) fn de_title<'de, D>(deserializer: D) -> Result<String, D::Error>
where
    D: serde::de::Deserializer<'de>,
{
    Ok(String::deserialize(deserializer)?.to_lowercase())
}

pub(crate) fn de_salary<'de, D>(deserializer: D) -> Result<Option<Salary>, D::Error>
where
    D: serde::de::Deserializer<'de>,
{
    #[derive(Deserialize, Debug)]
    pub(crate) struct Inner {
        from: Option<u64>,
        to: Option<u64>,
        currency: String,
    }

    type OptionalSalary = Option<Inner>;

    if let Some(salary) = OptionalSalary::deserialize(deserializer)? {
        if let Some(from) = salary.from {
            if let Some(to) = salary.to {
                let avg = (from + to) / 2;

                return Ok(Some(Salary {
                    avg: avg,
                    currency: salary.currency,
                }));
            }

            return Ok(Some(Salary {
                avg: from,
                currency: salary.currency,
            }));
        }

        if let Some(to) = salary.to {
            return Ok(Some(Salary {
                avg: to,
                currency: salary.currency,
            }));
        }
    }

    return Ok(None);
}

pub(crate) fn de_area<'de, D>(deserializer: D) -> Result<String, D::Error>
where
    D: serde::de::Deserializer<'de>,
{
    #[derive(Deserialize, Debug)]
    pub(crate) struct Area {
        name: String,
    }

    Ok(Area::deserialize(deserializer)?.name.to_lowercase())
}

pub(crate) fn de_published_at<'de, D>(deserializer: D) -> Result<DateTime<Utc>, D::Error>
where
    D: serde::de::Deserializer<'de>,
{
    Ok(String::deserialize(deserializer)?
        .parse::<DateTime<Utc>>()
        .map_err(serde::de::Error::custom)?)
}

pub(crate) fn de_schedule<'de, D>(deserializer: D) -> Result<bool, D::Error>
where
    D: serde::de::Deserializer<'de>,
{
    #[derive(Deserialize, Debug)]
    pub(crate) struct Schedule {
        id: String,
    }

    if let Ok(schedule) = Schedule::deserialize(deserializer) {
        return Ok(schedule.id == "remote");
    }

    return Ok(false);
}

pub(crate) fn de_specializations<'de, D>(deserializer: D) -> Result<Vec<Vec<String>>, D::Error>
where
    D: serde::de::Deserializer<'de>,
{
    #[derive(Deserialize, Debug)]
    pub(crate) struct Specialization {
        id: String,
        name: String,
    }

    type Specializations = Vec<Specialization>;

    Ok(Specializations::deserialize(deserializer)?
        .into_iter()
        .map(|s| vec![s.id, s.name.to_lowercase()])
        .collect())
}

pub(crate) fn de_skills<'de, D>(deserializer: D) -> Result<Vec<String>, D::Error>
where
    D: serde::de::Deserializer<'de>,
{
    #[derive(Deserialize)]
    struct Skill {
        pub name: String,
    }

    type Skills = Vec<Skill>;

    // TODO: rewrite this atrocity
    // prob. maintain kv map in fs
    // where k replacee token
    // and v is the replacement
    // load it up during runtime
    // and use to replace tokens
    // during each transformation cycle

    Ok(Skills::deserialize(deserializer)?
        .into_iter()
        .map(|s| s.name.to_lowercase())
        .map(|s| {
            if s.contains("1c") || s.contains("1с") {
                "1c".into()
            } else {
                s
            }
        })
        .map(|s| if s.contains("php") { "php".into() } else { s })
        .map(|s| if s.contains("css") { "css".into() } else { s })
        .map(|s| if s.contains("html") { "html".into() } else { s })
        .map(|s| if s.contains("rest") { "rest".into() } else { s })
        .map(|s| if s == "go" { "golang".into() } else { s })
        .map(|s| if s.contains("sap") { "sap".into() } else { s })
        .map(|s| {
            if s.contains("java") {
                if s.contains("script") {
                    "javascript".into()
                } else {
                    "java".into()
                }
            } else {
                s
            }
        })
        .map(|s| {
            if s.contains("js") {
                "javascript".into()
            } else {
                s
            }
        })
        .map(|s| {
            if s.contains("react") {
                if s.contains("native") {
                    "react native".into()
                } else {
                    "react".into()
                }
            } else {
                s
            }
        })
        .map(|s| {
            if s.contains("webpack") {
                "webpack".into()
            } else {
                s
            }
        })
        .map(|s| {
            if s.contains("front") {
                "front".into()
            } else {
                s
            }
        })
        .map(|s| if s.contains("back") { "back".into() } else { s })
        .map(|s| if s.contains("full") { "full".into() } else { s })
        .map(|s| if s.contains("lead") { "lead".into() } else { s })
        .map(|s| {
            if s.contains("junior") {
                "junior".into()
            } else {
                s
            }
        })
        .map(|s| {
            if s.contains("middle") {
                "middle".into()
            } else {
                s
            }
        })
        .map(|s| {
            if s.contains("senior") {
                "senior".into()
            } else {
                s
            }
        })
        .map(|s| {
            if s.contains("младший") {
                "младший".into()
            } else {
                s
            }
        })
        .map(|s| {
            if s.contains("старший") {
                "старший".into()
            } else {
                s
            }
        })
        .map(|s| {
            if s.contains("ведущий") {
                "ведущий".into()
            } else {
                s
            }
        })
        .collect())
}

pub(crate) fn de_experience<'de, D>(deserializer: D) -> Result<u8, D::Error>
where
    D: serde::de::Deserializer<'de>,
{
    #[derive(Deserialize, Debug)]
    pub(crate) struct Experience<'a> {
        id: &'a str,
    }

    if let Ok(experience) = Experience::deserialize(deserializer) {
        return Ok(match &experience.id {
            &"between1And3" => 1,
            &"between3And6" => 3,
            &"moreThan6" => 6,
            _ => 0,
        });
    }

    Ok(0)
}
