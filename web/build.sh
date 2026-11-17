#!/usr/bin/env bash
# Builds the WebAssembly module into web/dist/: orderbook.mjs (ES module factory) and orderbook.wasm.
# Needs em++ on PATH (emsdk: source emsdk_env.sh). It calls em++ directly rather than emcmake, which
# would configure the whole CMake project and fetch GoogleTest.
set -euo pipefail
cd "$(dirname "$0")/.."

src=OrderbookSim
mkdir -p web/dist
em++ --version | head -n 1

em++ -std=c++20 -O2 -fexceptions -DORDERBOOK_SINGLE_THREADED -I "$src" \
    web/orderbook_wasm.cpp \
    "$src/Orderbook.cpp" "$src/Order.cpp" "$src/Trade.cpp" "$src/OrderbookLevelInfos.cpp" \
    "$src/OrderDetailHistory.cpp" "$src/OrderDetail.cpp" "$src/NeuralNetwork.cpp" "$src/Net.cpp" "$src/Neuron.cpp" \
    -lembind -sMODULARIZE -sEXPORT_ES6 -sENVIRONMENT=web,node -sALLOW_MEMORY_GROWTH -sFILESYSTEM=0 \
    -o web/dist/orderbook.mjs

wc -c web/dist/orderbook.mjs web/dist/orderbook.wasm
