use chrono::{DateTime, Utc};
use serde::Deserialize;

#[derive(Deserialize, Debug)]
pub(crate) struct Salary {
    avg: u64,
    currency: String,
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

    let salary = OptionalSalary::deserialize(deserializer)?;

    if let Some(salary) = salary {
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

    let area = Area::deserialize(deserializer)?;

    Ok(area.name)
}

pub(crate) fn de_published_at<'de, D>(deserializer: D) -> Result<String, D::Error>
where
    D: serde::de::Deserializer<'de>,
{
    let s = String::deserialize(deserializer)?;

    let date = s
        .parse::<DateTime<Utc>>()
        .map_err(serde::de::Error::custom)?;

    Ok(date.format("%Y-%m-%dT%H:%M:%S").to_string())
}

pub(crate) fn de_schedule<'de, D>(deserializer: D) -> Result<bool, D::Error>
where
    D: serde::de::Deserializer<'de>,
{
    #[derive(Deserialize, Debug)]
    pub(crate) struct Schedule {
        id: String,
    }

    let schedule = Schedule::deserialize(deserializer);

    if let Ok(schedule) = schedule {
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

    let specs = Specializations::deserialize(deserializer)?;

    Ok(specs.into_iter().map(|s| vec![s.id, s.name]).collect())
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

    let skills = Skills::deserialize(deserializer)?;

    Ok(skills.into_iter().map(|s| s.name).collect())
}

pub(crate) fn de_experience<'de, D>(deserializer: D) -> Result<u8, D::Error>
where
    D: serde::de::Deserializer<'de>,
{
    #[derive(Deserialize, Debug)]
    pub(crate) struct Experience<'a> {
        id: &'a str,
    }

    let experience = Experience::deserialize(deserializer);

    if let Ok(experience) = experience {
        return Ok(match &experience.id {
            &"between1And3" => 1,
            &"between3And6" => 3,
            &"moreThan6" => 6,
            _ => 0,
        });
    }

    Ok(0)
}
