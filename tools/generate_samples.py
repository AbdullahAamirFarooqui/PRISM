"""
Generates the sample datasets in ../data (synthetic but realistic-looking).
Run:  python tools/generate_samples.py
"""
import csv
import math
import os
import random
from datetime import date, timedelta

OUT = os.path.join(os.path.dirname(__file__), "..", "data")
os.makedirs(OUT, exist_ok=True)


def write(name, header, rows):
    with open(os.path.join(OUT, name), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(header)
        w.writerows(rows)
    print("wrote", name, len(rows), "rows")


def weather_rows(year, seed, base_temp):
    rnd = random.Random(seed)
    rows = []
    cities = ["Karachi", "Lahore", "Islamabad"]
    start = date(year, 1, 1)
    for d in range(365):
        day = start + timedelta(days=d)
        season = math.sin((d - 100) / 365 * 2 * math.pi)       # peak in summer
        for city in cities:
            offset = {"Karachi": 3, "Lahore": 1, "Islamabad": -3}[city]
            temp = base_temp + offset + 9 * season + rnd.gauss(0, 1.8)
            humidity = max(15, min(100, 75 - 1.4 * (temp - 25) + rnd.gauss(0, 6) + (10 if city == "Karachi" else 0)))
            pressure = 1013 - 0.55 * (temp - 25) + rnd.gauss(0, 2.5)
            wind = max(0, 12 + rnd.gauss(0, 4) + (5 if city == "Karachi" else 0))
            rain = max(0.0, (humidity - 70) * 0.45 + rnd.gauss(0, 3)) if rnd.random() < 0.45 else 0.0
            uv = max(0, min(12, 2 + 0.3 * (temp - 10) + rnd.gauss(0, 0.8)))
            rows.append([day.isoformat(), city, round(temp, 1), round(humidity, 1), round(pressure, 1),
                         round(wind, 1), round(rain, 1), round(uv, 1)])
    return rows


WEATHER_HEADER = ["Date", "City", "Temperature", "Humidity", "Pressure", "WindSpeed", "Rainfall", "UVIndex"]


def make_messy(rows, seed, numeric_cols, text_cols):
    """Adds missing values, invalid values and duplicate rows."""
    rnd = random.Random(seed)
    messy = [list(r) for r in rows]
    for r in messy:
        for c in numeric_cols:
            p = rnd.random()
            if p < 0.03:
                r[c] = rnd.choice(["", "NA", "null", "?"])
            elif p < 0.045:
                v = r[c]
                r[c] = rnd.choice([f"{v} units", f"#{v}", "error", "n.a", f"{v}C" if isinstance(v, float) else f"{v}!"])
        for c in text_cols:
            if rnd.random() < 0.02:
                r[c] = ""
    for _ in range(len(messy) // 25):                          # ~4% duplicates
        messy.insert(rnd.randrange(len(messy)), list(rnd.choice(messy)))
    return messy


# ---------------- weather ----------------
w2025 = weather_rows(2025, 1, 25)
w2024 = weather_rows(2024, 2, 24.2)
write("weather_2025.csv", WEATHER_HEADER, w2025)
write("weather_2024.csv", WEATHER_HEADER, w2024)
write("weather_messy.csv", WEATHER_HEADER, make_messy(w2025, 3, [2, 3, 4, 5, 6, 7], [1]))

# ---------------- earthquakes ----------------
rnd = random.Random(10)
regions = [("Pacific Ring", -10, 150), ("Himalaya", 30, 80), ("Andes", -20, -70),
           ("Mediterranean", 38, 22), ("Japan Trench", 37, 143), ("Alaska", 60, -150)]
quakes = []
start = date(2020, 1, 1)
for i in range(1500):
    name, lat, lon = rnd.choice(regions)
    mag = round(min(9.1, 3.0 + rnd.expovariate(1.1)), 1)
    depth = round(max(1, rnd.gauss(35, 25) + (mag - 4) * 6), 1)
    energy = round(10 ** (1.5 * mag + 4.8) / 1e12, 3)          # terajoules
    felt = int(max(0, (mag - 3.0) ** 2.6 * 40 / (1 + depth / 60) + rnd.gauss(0, 20)))
    tsunami = 1 if (mag > 7 and name in ("Pacific Ring", "Japan Trench", "Andes") and rnd.random() < 0.6) else 0
    day = start + timedelta(days=rnd.randrange(365 * 5))
    quakes.append([day.isoformat(), name, round(lat + rnd.gauss(0, 4), 3), round(lon + rnd.gauss(0, 5), 3),
                   depth, mag, energy, felt, tsunami])
quakes.sort(key=lambda r: r[0])
Q_HEADER = ["Date", "Region", "Latitude", "Longitude", "Depth_km", "Magnitude", "Energy_TJ", "FeltReports", "Tsunami"]
write("earthquakes.csv", Q_HEADER, quakes)
write("earthquakes_messy.csv", Q_HEADER, make_messy(quakes, 11, [2, 3, 4, 5, 6, 7], [1]))

# ---------------- population ----------------
rnd = random.Random(20)
countries = {"Pakistan": (108, 3.1), "India": (870, 2.1), "Bangladesh": (106, 2.4), "China": (1135, 1.5),
             "Nigeria": (95, 2.7), "Brazil": (150, 1.6), "Indonesia": (181, 1.8), "Egypt": (57, 2.2)}
pop = []
for country, (p0, growth) in countries.items():
    p = p0
    life = rnd.uniform(55, 68)
    urban = rnd.uniform(20, 50)
    gdp = rnd.uniform(300, 2000)
    for year in range(1990, 2025):
        births = max(8, 42 - 0.45 * (year - 1990) * growth / 2 - (gdp / 1000) + rnd.gauss(0, 1))
        deaths = max(4, 12 - 0.12 * (year - 1990) + rnd.gauss(0, 0.4))
        p *= 1 + (births - deaths) / 1000
        life += 0.25 + rnd.gauss(0, 0.15)
        urban += 0.35 + rnd.gauss(0, 0.2)
        gdp *= 1.04 + rnd.gauss(0, 0.03)
        literacy = min(99, 30 + 0.8 * urban + rnd.gauss(0, 2))
        pop.append([country, year, round(p, 2), round(births, 2), round(deaths, 2), round(life, 1),
                    round(urban, 1), round(gdp, 0), round(literacy, 1)])
P_HEADER = ["Country", "Year", "Population_M", "BirthRate", "DeathRate", "LifeExpectancy",
            "UrbanPercent", "GDP_per_capita", "LiteracyRate"]
write("population.csv", P_HEADER, pop)

# ---------------- rainfall ----------------
rnd = random.Random(30)
rain = []
for region, base in [("Sindh", 15), ("Punjab", 40), ("KPK", 60), ("Balochistan", 10), ("Gilgit", 30)]:
    for year in range(2000, 2025):
        for month in range(1, 13):
            monsoon = 1 + 5 * math.exp(-((month - 7.5) ** 2) / 2)
            rainfall = max(0, base * monsoon + rnd.gauss(0, base * 0.5))
            temp = 22 + 10 * math.sin((month - 4) / 12 * 2 * math.pi) + rnd.gauss(0, 1.2) - (8 if region == "Gilgit" else 0)
            humidity = min(100, 35 + rainfall * 0.25 + rnd.gauss(0, 5))
            river = round(2 + rainfall * 0.03 + rnd.gauss(0, 0.3), 2)
            crop = round(max(0, 1.5 + rainfall * 0.01 - abs(temp - 25) * 0.05 + rnd.gauss(0, 0.2)), 2)
            rain.append([year, month, region, round(rainfall, 1), round(temp, 1), round(humidity, 1), river, crop])
R_HEADER = ["Year", "Month", "Region", "Rainfall_mm", "AvgTemp", "Humidity", "RiverLevel_m", "CropYield_t_per_ha"]
write("rainfall.csv", R_HEADER, rain)
write("rainfall_messy.csv", R_HEADER, make_messy(rain, 31, [3, 4, 5, 6, 7], [2]))
