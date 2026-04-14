# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

`smartmet-library-locus` is a C++ geocoding library that resolves place names to coordinates with multilingual support (ISO 639 language codes). It queries a PostgreSQL/PostGIS `fminames` database (GeoNames-derived). Used by `smartmet-engine-geonames` and `smartmet-plugin-autocomplete`.

## Build Commands

```bash
make                  # Build libsmartmet-locus.so
make test             # Run all tests (requires PostgreSQL fminames database)
make format           # Run clang-format on all source and test files
make clean            # Clean build artifacts
make rpm              # Build RPM package
make install          # Install library and headers
```

## Testing

Tests require a running PostgreSQL database with GeoNames data:
- **Local development**: connects to `smartmet-test:5444` with user `fminames_user`, database `fminames`
- **CI**: creates a temporary local PostgreSQL instance via `create-local-db.sh`

Tests use the `regression/tframe.h` framework (not Boost.Test). Each test file compiles to a standalone executable:

```bash
cd test && make test                    # Run all tests
cd test && make QueryTest && ./QueryTest  # Build and run a single test
```

Test executables: `QueryTest`, `QueryOptionsTest`, `ISOI639Test`.

## Architecture

All code is in the `Locus` namespace with four classes:

- **`Query`** — main interface. Connects to PostgreSQL and provides `FetchByName`, `FetchByLatLon`, `FetchByLonLat`, `FetchById`, `FetchByKeyword`. Constructs SQL with parameterized builders (`buildFetchByNameSQL`, etc.) and uses `pqxx::connection` via `Fmi::Database::PostgreSQLConnection`. Also manages a shared `ISO639` table loaded from the database.
- **`QueryOptions`** — search parameters: country/feature filters, language, charset, collation, result limits, population filters, autocomplete mode, name type (fmisid/wmo/lpnn). Defaults to Finnish (`fi`) language and country.
- **`SimpleLocation`** — plain data struct for results: name, lat/lon, country, feature code, timezone, population, iso2, elevation, optional fmisid.
- **`ISO639`** — maps between ISO 639-1/2/3 language codes. Loaded from the `fminames` database or populated manually. Supports special codes (wmo, fmisid, lpnn).

## Dependencies

- `smartmet-library-macgyver` (PostgreSQLConnection, StringConversion, TypeTraits)
- `libpqxx` (PostgreSQL C++ client)
- Boost (locale, regex, thread)
- ICU (`icu-i18n` for Unicode operations)
