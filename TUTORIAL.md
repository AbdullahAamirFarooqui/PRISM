# PRISM Tutorial – Step by Step

This guide walks you through each screen of PRISM using a **simple sales dataset** (instead of the big weather file).

---

## Setup: Load the Simple Dataset

1. **App is open** → click **"Load sample"**
2. **Look for `simple_sales`** → click it
3. **Dashboard appears** showing:
   - 25 rows (products sold)
   - 5 columns (Product, Month, Sales, Profit, Category)
   - 0 missing values (it's clean!)
   - 0 invalid values
   - 0 duplicates

You're ready to explore!

---

## Screen 1: Dashboard

**What it shows:**
- A summary of your dataset
- 6 cards: rows, columns, numeric columns, missing values, invalid values, duplicates
- A table of data quality metrics
- A progress bar for "batch analysis" (auto-analysis running in background)

**In our simple data:**
```
Rows:                25
Columns:             5
Numeric columns:     3  (Sales, Profit, and Month as a number)
Text columns:        2  (Product, Category)
Missing values:      0  (clean!)
Invalid values:      0  (clean!)
Duplicate rows:      0  (clean!)
```

**What you do:**
- Look at the summary to understand your data at a glance
- Click **"Run full analysis"** (if not already running) to compute stats, correlations, and Top-5 in the background

---

## Screen 2: Dataset (the table view)

**Click the "Dataset" tab.**

You now see the actual table:

| Product  | Month | Sales | Profit | Category      |
|----------|-------|-------|--------|---------------|
| Laptop   | Jan   | 5000  | 1200   | Electronics   |
| Laptop   | Feb   | 4800  | 1150   | Electronics   |
| Laptop   | Mar   | 5200  | 1300   | Electronics   |
| Mouse    | Jan   | 500   | 150    | Electronics   |
| ...      | ...   | ...   | ...    | ...           |

**On the right side: Cleaning tools**

### Remove duplicate rows
- We have no duplicates, so this button does nothing
- But if you had two identical rows, click this to delete all copies except one

### Handle missing values
- Our data is complete (no red cells)
- If we had empty cells, you'd choose:
  - **Replace with mean** → fill with the average value
  - **Replace with median** → fill with the middle value
  - **Replace with zero** → fill with 0
  - **Remove rows** → delete entire row if it has a missing value

### Fix invalid values
- Our data is valid (no orange cells)
- If we had `"5,000"` (with comma) or `"$5000"` (with dollar sign), this would clean it

### Trim whitespace
- Removes leading/trailing spaces from text

---

## Screen 3: Statistics

**Click the "Statistics" tab.**

### Left side: Column Statistics
Pick a column (e.g., **Sales**) from the dropdown. You see:

| Statistic | Value | Meaning |
|---|---|---|
| Count | 9 | 9 sales records (9 rows where Product='Laptop' or similar) |
| Mean | 4700 | average sales = 4700 |
| Median | 4800 | middle value = 4800 |
| Mode | 5000 | most frequent value = 5000 (appears 3 times) |
| Min | 4800 | smallest sale = 4800 |
| Max | 5200 | largest sale = 5200 |
| Range | 400 | max - min = 400 |
| Variance | 41667 | spread of data |
| Std Dev | 204 | standard deviation (square root of variance) |
| Q1 | 4800 | 25th percentile |
| Q3 | 5150 | 75th percentile |
| IQR | 350 | interquartile range (Q3 - Q1) |

### Middle: Histogram
A bar chart showing how many sales fall in each range:
- 4800–4900: 3 sales
- 4900–5000: 2 sales
- 5000–5100: 2 sales
- 5100–5200: 2 sales

### Bottom: Frequency table
Same histogram data as a table with counts and percentages.

### Right: Correlation matrix
Shows how related columns are:
- **Sales vs Profit:** 0.95 (very related! more sales = more profit)
- **Sales vs Month:** -0.10 (barely related)
- Color coding: green = positive, red = negative, brighter = stronger

---

## Screen 4: Charts

**Click the "Charts" tab.**

Four chart types. Let's try each:

### Line Chart
- **Column:** Sales
- **Group by (optional):** Category
- **Update chart**

Result: 3 lines (one per category: Electronics, Furniture, Stationery), showing sales over months (Jan, Feb, Mar).

### Bar Chart
- **Column:** Profit
- **Group by:** Category
- **Update chart**

Result: 3 bars showing average profit per category:
- Electronics: ~437
- Furniture: ~270
- Stationery: ~58

### Scatter Plot
- **X axis:** Sales
- **Y axis:** Profit
- **Update chart**

Result: points scattered, showing the relationship. A trend line (y = mx + b) appears if there's a correlation. You see the equation and the Pearson r value.

### Histogram
- **Column:** Sales
- **Bins:** 6
- **Update chart**

Result: bars showing how many sales fall in each range (same as Statistics tab).

---

## Screen 5: Compare

**Click the "Compare" tab.**

This screen lets you compare two datasets side-by-side.

1. Click **"Load comparison dataset"**
2. Pick a different CSV (e.g., `simple_sales` again, or another file)
3. You'll see:
   - A statistics table comparing the two datasets
   - An overlaid line chart showing both

**Use case:** compare sales from Jan vs Feb, or this year vs last year.

---

## Screen 6: Algorithm Lab

**Click the "Algorithm Lab" tab.**

Four sub-tabs showing algorithms in action. These are **educational**, showing how sorting and searching work.

### Tab 1: Sorting benchmark
- **Column:** Sales (or any numeric column)
- **Elements:** 25 (how many values to sort)
- **Input order:** choose one:
  - **Dataset order** (random-ish)
  - **Sorted** (already sorted, will be fast)
  - **Reverse sorted** (worst case for some algorithms)
- **Run benchmark**

Result: a table showing 6 sorting algorithms:

| Algorithm | Complexity | n | Comparisons | Swaps | Time (ms) | Sorted? |
|---|---|---|---|---|---|---|
| Bubble Sort | O(n²) | 25 | 300 | 47 | 0.05 | yes |
| Selection Sort | O(n²) | 25 | 300 | 25 | 0.03 | yes |
| Insertion Sort | O(n²) | 25 | 150 | 47 | 0.02 | yes |
| Merge Sort | O(n log n) | 25 | 112 | 50 | 0.01 | yes |
| Quick Sort | O(n log n) avg | 25 | 120 | 35 | 0.01 | yes |
| Heap Sort | O(n log n) | 25 | 130 | 60 | 0.02 | yes |

**Learn:** Bubble Sort is slow (300 comparisons), Quick Sort is fast (120 comparisons).

### Tab 2: Searching
- **Column:** Product
- **Value:** e.g., "Laptop"
- **Pick existing value** (or type one)
- **Search**

Result: comparison table:

| Algorithm | Works on | Found? | Position | Comparisons | Time (µs) |
|---|---|---|---|---|---|
| Linear Search | unsorted array | yes | index 0 | 1 | 0.30 |
| Binary Search | sorted array | yes | (sorted pos) | 4 | 0.20 |
| Hash Search | hash table | yes | 7 matches | 2 | 0.40 |

**Learn:** Linear Search checks every item; Binary Search skips half each time; Hash Search looks up by key.

### Tab 3: Top-K (heap)
- **Column:** Profit
- **K:** 5
- **Largest** (or Smallest)
- **Rank**

Result: Top 5 products by profit:

| Rank | Product | Profit | Row | ... |
|---|---|---|---|---|
| 1 | Laptop | 1300 | 3 | ... |
| 2 | Monitor | 530 | 20 | ... |
| 3 | Keyboard | 220 | 23 | ... |
| 4 | Chair | 240 | 12 | ... |
| 5 | Desk | 350 | 8 | ... |

**Learn:** Heap is faster than sorting when K << n (e.g., finding top 5 of 1000).

### Tab 4: BST visualizer
- **Column:** Sales
- **Values:** 15 (how many to insert)
- **Insert order:**
  - **Dataset order** (random)
  - **Sorted order** (will show bad tree!)
  - **Random** (good tree)
- **Build tree**

Then:
- Type a value (e.g., 4800)
- Click **"Search (animate)"**
- Watch the tree nodes highlight in order as the search walks down

**Learn:** a balanced tree is fast, a sorted-input tree becomes a linked list (slow).

---

## Screen 7: Relationship Graph

**Click the "Relationship Graph" tab.**

Shows columns as **nodes** and correlations as **edges**.

### Controls:
- **Edge when |r| >=** threshold (default 0.70)
  - Higher = only show very strong correlations
  - Lower = show weaker ones too
- **Start column:** pick where to start the traversal
- **Run BFS** / **Run DFS** buttons

### What you see:
- **Nodes** (circles) = columns (Sales, Profit, Category, etc.)
- **Edges** (lines) = correlations:
  - **Blue** = positive (move together)
  - **Red** = negative (move opposite)
  - **Thickness** = strength of correlation
- **Node colour** = cluster (connected component)

### Animation:
Click **Run BFS** from "Sales":
- Nodes light up yellow one by one
- Text on the right shows: "1. visit Sales, 2. visit Profit, ..." (the order visited)
- Shows which columns are reachable from Sales via correlations

**Learn:** Graph algorithms like BFS/DFS traverse data relationships.

---

## Screen 8: History Panel (right side)

**Always visible** on the right: "History (linked list)"

Shows every operation you did, **newest first**:
```
19:20:32  Top-10 largest values of 'Profit'
19:20:12  Sorting benchmark on 'Sales' (n=25, Dataset order)
19:20:00  Loaded comparison dataset
```

**What it means:**
- Each operation is timestamped
- Click to see details
- Everything is **undoable** (Ctrl+Z)

---

## File Menu: Save & Export

**Click "File" menu** at the top.

### Save Project (.sda)
- Saves your dataset, all changes, and history to a `.sda` file
- Reopen it later to continue where you left off

### Export Report (HTML)
- Generates a pretty HTML file with:
  - Data quality summary
  - All statistics
  - Correlation heat map
  - Histograms
- Open in any browser to share with others

### Export Cleaned CSV
- Download the dataset as a `.csv` file
- If you cleaned it (removed duplicates, fixed invalid values), this is the clean version

---

## Undo/Redo

**Ctrl+Z** = undo last action
**Ctrl+Y** = redo

Try it: remove duplicates, then Ctrl+Z to bring them back!

---

## Summary: What Each Tab Does

| Tab | Shows | You can |
|---|---|---|
| Dashboard | Summary cards | see data quality at a glance |
| Dataset | The table | clean data, search, find ranges |
| Statistics | Descriptive stats | understand one column deeply |
| Charts | 4 types of plots | visualize relationships |
| Compare | Side-by-side stats | compare two datasets |
| Algorithm Lab | 4 algorithms | learn how sorting/searching work |
| Relationship Graph | Column network | see which columns are related |

---

## Try This Right Now

1. **Open `simple_sales`** (if not already open)
2. **Go to Charts** → Line chart, Sales, Group by Category → click Update
3. **Watch:** 3 lines showing how Electronics, Furniture, and Stationery sales changed over Jan–Mar
4. **Go to Algorithm Lab** → Sorting → Column: Profit → Input order: Sorted → Run benchmark
5. **Watch:** Bubble Sort does 300 comparisons, Quick Sort does 120
6. **Go to Statistics** → Column: Profit → see mean, median, etc.
7. **Ctrl+Z** a few times → watch the history undo actions

You've now explored all 7 screens! 🎉

