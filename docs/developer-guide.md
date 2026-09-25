# locus developer guide

This guide is for developers who change `smartmet-library-locus`, or use it. locus finds
places in the `fminames` PostgreSQL/PostGIS database (GeoNames data plus FMI additions):
by name, by coordinates, by id and by keyword, with names in the requested language. The
geonames engine uses it for every database lookup; plugins reach it through the engine.

[CLAUDE.md](../CLAUDE.md) has the build summary.

## Contents

1. [Building and testing](#1-building-and-testing)
2. [The API](#2-the-api)
3. [Options](#3-options)
4. [How a name search works](#4-how-a-name-search-works)
5. [Coordinate, id and keyword searches](#5-coordinate-id-and-keyword-searches)
6. [Languages](#6-languages)
7. [Threads and connections](#7-threads-and-connections)
8. [Compatibility](#8-compatibility)
9. [Known pitfalls](#9-known-pitfalls)

---

## 1. Building and testing

```bash
make
make test                                # needs the fminames database
make -C test QueryTest && ./test/QueryTest
```

The tests connect to `smartmet-test:5444` (database `fminames`, user `fminames_user`).
With `CI` set, the test Makefile creates a temporary local database with
`/usr/share/smartmet/test/db/create-local-db.sh` and connects through its socket
directory instead.

## 2. The API

```cpp
Locus::Query q(host, user, password, database, port);
q.load_iso639_table();                       // once per process, see §6

Locus::QueryOptions opts;
opts.SetLanguage("en");
auto places = q.FetchByName(opts, "Helsinki");
```

| Call | Returns |
|------|---------|
| `FetchByName(opts, name)` | Places whose name, or a name variant, matches (§4). |
| `FetchByLonLat(opts, lon, lat, radius)`, `FetchByLatLon(opts, lat, lon, radius)` | Places nearest to the point, within `radius` km (default 50) (§5). |
| `FetchById(opts, id)` | The place with this geonames id. |
| `FetchByKeyword(opts, keyword)`, `CountKeywordLocations(opts, keyword)` | The places of a keyword (a named list of places, such as a set of stations or cities). |
| `cancel()` | Cancels the running statement (from another thread). |

Results are `SimpleLocation`s: name (in the requested language), lon, lat, country name,
feature code and its description, administrative area (municipality, or the first-level
administrative division), time zone, population, ISO 3166 country code, id, elevation (or
the DEM height when the elevation is missing) and the FMI station id when one exists.
Places without a time zone are never returned.

## 3. Options

`QueryOptions` defaults:

| Option | Default | Meaning |
|--------|---------|---------|
| `SetCountries()` | `fi` | Country codes to search, comma separated or a list. `all` or `%` means every country. The order also sets the country priority of the results. |
| `SetExcludedCountries()` | none | Countries to leave out (but see the pitfalls). |
| `SetFeatures()` | `PPLC,ADMD,PPLA,…,PRK` | GeoNames feature codes to search; the order sets the priority. `all` or `%` means every feature. |
| `SetKeywords()` | none | Restrict to places of these keywords. |
| `SetLanguage()` | `fi` | Language of the returned names. |
| `SetNameType()` | empty | `fmisid`, `wmo` or `lpnn`: return the station id of that type as the name. |
| `SetResultLimit()` | 100 | Maximum number of results; 0 means no limit. |
| `SetFullCountrySearch()` | false | If nothing is found in the given countries, search all countries. |
| `SetSearchVariants()` | true | Also match alternate names (other languages, old names). |
| `SetAutocompleteMode()` | false | Autocomplete behaviour (§4). |
| `SetPopulationMin()`, `SetPopulationMax()` | 0 (off) | Population limits. |
| `SetCharset()` | `utf8` | Result character set; other than UTF-8 converts names, falling back to the ASCII name. |

`HashValue()` covers every option, and the geonames engine uses it with the name as its
cache key. The older string `Hash()` is deprecated and leaves out the keywords.

## 4. How a name search works

1. The name is split at the first comma: `Helsinki,Suomi` searches for `Helsinki`, and then
   keeps only the results whose country name or administrative area is `Suomi`, compared
   in lower case. The country and area names are in the result language.
2. The search word is used as a SQL `LIKE` pattern, compared in lower case. `%` and `_`
   are therefore wildcards: `Hel%` matches every name starting with `Hel`.
3. With `SetSearchVariants(true)`, the query is the union of a match on the main name and a
   match on the alternate names of the result language.
4. The results are ordered by the database priority, then by population for places over
   50 000 people, the country priority, the feature priority, population and name.
5. For each result, the name in the requested language is looked up among the alternate
   names (preferring priority, preferred names and short names), together with the
   country, feature, administrative area names and FMI station ids, with one query each
   for the whole result set.
6. The result limit is applied while building the results. In autocomplete mode, exact
   matches of the search word (without its trailing `%`) are then moved first.
7. If nothing was found and `SetFullCountrySearch(true)` is set, the search is repeated
   with every country.

## 5. Coordinate, id and keyword searches

* **Coordinates.** `FetchByLonLat()` finds candidates with the PostGIS KNN operator
  (`the_geom <-> point`), limited to the result limit plus 10, computes the geodesic
  distance, and keeps those within the radius, nearest first. A radius of 0 means no
  distance limit.
* **Id.** `FetchById()` looks up the id; if nothing is found and the id is 10 000 000 or
  more, it tries the negative id, which is how FMI's own additions are numbered.
* **Keywords.** `FetchByKeyword()` checks that the keyword exists and returns all its
  places, ignoring the result limit. A keyword can override the displayed name of a place.

## 6. Languages

Language codes are resolved through an ISO 639 table (`ISO639`, loaded from the
database), which maps ISO 639-1, -2 and -3 codes to each other: `fi` and `fin` mean the
same language, and the query then matches every code of it in
`alternate_geonames.language`. The table is process-wide and **empty until
`load_iso639_table()` is called**; before that, the language code is matched as given.
The optional special codes (`fmisid`, `wmo`, `lpnn`, …) are added to the table as
languages of their own, since the station ids are stored as alternate names.
`load_iso639_table()` replaces the table atomically, so it is safe to call while other
threads search.

## 7. Threads and connections

A `Query` owns one database connection (`Fmi::Database::PostgreSQLConnection`) and holds
per-search state, so **one `Query` may be used by one thread at a time**. The geonames
engine keeps a pool of them. The connection's reconnect behaviour (waits and retries when
the database goes away) comes from macgyver; see the macgyver developer guide.

## 8. Compatibility

The headers are installed and used by the geonames engine and the autocomplete plugin.
`QueryOptions` and `SimpleLocation` are copied and cached by the engine, so a change to
their layout requires rebuilding the engine. When adding an option, add it to
`HashValue()` too, or cached results will be reused for different options.

## 9. Known pitfalls

* **No space after the comma.** `Helsinki,Suomi` works, but `Helsinki, Suomi` finds
  nothing: the area part keeps its leading space and matches no country or area name.
* **`SetExcludedCountries()` has no effect.** The excluded codes are compared in lower
  case, while the database stores upper-case codes, and the exclusion is also skipped when
  every country is searched.
* **The search word is a `LIKE` pattern.** A name containing `%` or `_` matches more than
  itself.
* **Autocomplete mode assumes a trailing `%`.** The exact-match sorting drops the last
  character of the search word.
* **The default country is Finland.** A plain `FetchByName()` for a foreign place finds
  nothing unless the countries are set or `SetFullCountrySearch(true)` is used.
* **Call `load_iso639_table()`**, or language aliases (`fin`, `swe`, …) do not work.
* **A radius of 0 is unlimited** in coordinate searches.
