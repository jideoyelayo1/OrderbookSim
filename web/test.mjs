// Parity test: replays a seeded 20,000-call flow through the WebAssembly Book (web/dist, from
// web/build.sh) and through the native build of the same Book (orderbook_parity, from CMake) and
// requires identical output. The network read-outs are the exception: they start from rand()
// weights and see the wall clock, so they are only required to be finite on both sides.
// Both sides share Book.hpp, so the flow's trade and outcome counts are pinned as well, and a
// WebAssembly-only section checks how the binding treats arguments of the wrong JavaScript type.
//
// Usage, from the repository root: node web/test.mjs [path/to/orderbook_parity]
// No dependencies beyond Node.

import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';
import createModule from './dist/orderbook.mjs';

const CALLS = 20000;
const SEED = 20241117;
const driver = process.argv[2] ?? fileURLToPath(new URL('../build/orderbook_parity', import.meta.url));

// mulberry32: small seeded generator, so the flow is the same on every run and machine
function generator(seed) {
  return () => {
    seed = (seed + 0x6d2b79f5) | 0;
    let t = Math.imul(seed ^ (seed >>> 15), 1 | seed);
    t = (t + Math.imul(t ^ (t >>> 7), 61 | t)) ^ t;
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
}

function flow() {
  const random = generator(SEED);
  const pick = (n) => Math.floor(random() * n);
  const calls = [];
  let ids = 0; // valid adds since the last reset; Book numbers them 1..ids
  while (calls.length < CALLS - 2) {
    const r = pick(1000);
    const side = pick(2) ? 'Buy' : 'Sell';
    const price = 95 + pick(11);
    const qty = 1 + pick(20);
    const id = Math.max(1, ids - pick(40)); // recent ids, which are more often still resting
    if (calls.length % 5000 === 4999) {
      calls.push('reset');
      ids = 0;
    } else if (r < 150) calls.push(`cancel ${id}`);
    else if (r < 200) calls.push(`modify ${id} ${side} ${price} ${qty}`);
    else if (r < 205) calls.push('eod');
    else if (r < 215) calls.push(['add Limit Buy 100 5', `add GoodTillCancel ${side} ${price} 0`, `add FillOrKill ${side} ${price} 2.5`, `add GoodForDay ${side} 0 ${qty}`][pick(4)]);
    else if (r < 235) calls.push('levels');
    else if (r < 240) calls.push('readouts');
    else {
      const type = r < 600 ? 'GoodTillCancel' : r < 700 ? 'GoodForDay' : r < 800 ? 'FillAndKill' : r < 900 ? 'FillOrKill' : 'Market';
      calls.push(`add ${type} ${side} ${type === 'Market' ? 0 : price} ${qty}`);
      ids++;
    }
  }
  calls.push('levels', 'readouts');
  return calls;
}

const tradesText = (trades) =>
  trades.length ? trades.map((t) => `${t.bidId}:${t.bidPrice}/${t.askId}:${t.askPrice}/${t.quantity}`).join(',') : '-';
const levelsText = (levels) => (levels.length ? levels.map((l) => `${l.price}:${l.quantity}`).join(',') : '-');
const resultText = (call, r) => `${call} ${r.id} ${r.status} ${r.restingQuantity} ${tradesText(r.trades)}`;

function run(book, line) {
  const [call, ...a] = line.split(' ');
  switch (call) {
    case 'add': return resultText(call, book.add(a[0], a[1], Number(a[2]), Number(a[3])));
    case 'modify': return resultText(call, book.modify(Number(a[0]), a[1], Number(a[2]), Number(a[3])));
    case 'cancel': return `cancel ${book.cancel(Number(a[0]))}`;
    case 'eod': book.endOfDay(); return `eod ${book.size()}`;
    case 'reset': book.reset(); return 'reset';
    case 'levels': { const l = book.levels(); return `levels ${levelsText(l.bids)} | ${levelsText(l.asks)}`; }
    case 'readouts': return `readouts ${book.size()} ${book.vwap()} network ${book.prediction()} ${book.recentError()}`;
    default: throw new Error(`unknown call: ${line}`);
  }
}

const calls = flow();
const native = spawnSync(driver, { input: calls.join('\n') + '\n', encoding: 'utf8', maxBuffer: 1 << 28 });
if (native.error || native.status !== 0) {
  console.error(`native driver ${driver} failed:`, native.error?.message ?? native.stderr);
  process.exit(1);
}
const expected = native.stdout.trimEnd().split('\n');

const Module = await createModule();
const book = new Module.Book();
const actual = calls.map((line) => run(book, line));
book.delete();

let failures = 0;
const fail = (message) => {
  if (++failures <= 10) console.error(message);
};
if (expected.length !== calls.length) fail(`native printed ${expected.length} lines for ${calls.length} calls`);

const outcomes = {};
let trades = 0;
for (let i = 0; i < calls.length; i++) {
  const [mine, theirs] = [actual[i], expected[i] ?? ''];
  if (mine.startsWith('readouts ')) {
    const [head, network] = mine.split(' network ');
    const [nativeHead, nativeNetwork = ''] = theirs.split(' network ');
    if (head !== nativeHead) fail(`call ${i} (${calls[i]}):\n  wasm   ${head}\n  native ${nativeHead}`);
    for (const value of [...network.split(' '), ...nativeNetwork.split(' ')])
      if (!Number.isFinite(Number(value)) || value === '') fail(`call ${i}: network read-out ${JSON.stringify(value)} is not finite`);
    continue;
  }
  if (mine !== theirs) fail(`call ${i} (${calls[i]}):\n  wasm   ${mine}\n  native ${theirs}`);
  const [call, , status, , tradeList] = mine.split(' ');
  if (call === 'add' || call === 'modify') {
    const key = `${call === 'add' ? calls[i].split(' ')[1] : 'modify'} ${status}`;
    outcomes[key] = (outcomes[key] ?? 0) + 1;
    if (tradeList !== '-') trades += tradeList.split(',').length;
  }
}

// The flow is fixed by SEED, so these are too; a change in Book that both builds share shows here.
const EXPECTED_TRADES = 10745;
const EXPECTED_OUTCOMES = {
  'FillAndKill filled': 589, 'FillAndKill partial': 212, 'FillAndKill rejected': 1263,
  'FillOrKill filled': 516, 'FillOrKill invalid': 39, 'FillOrKill rejected': 1459,
  'GoodForDay filled': 516, 'GoodForDay invalid': 47, 'GoodForDay partial': 203, 'GoodForDay resting': 1294,
  'GoodTillCancel filled': 2060, 'GoodTillCancel invalid': 47, 'GoodTillCancel partial': 743, 'GoodTillCancel resting': 4328,
  'Limit invalid': 59,
  'Market filled': 1641, 'Market partial': 161, 'Market rejected': 195,
  'modify filled': 42, 'modify partial': 12, 'modify rejected': 879, 'modify resting': 93,
};
if (trades !== EXPECTED_TRADES) fail(`trades: ${trades}, expected ${EXPECTED_TRADES}`);
for (const key of new Set([...Object.keys(EXPECTED_OUTCOMES), ...Object.keys(outcomes)]))
  if (outcomes[key] !== EXPECTED_OUTCOMES[key]) fail(`outcome ${key}: ${outcomes[key] ?? 0}, expected ${EXPECTED_OUTCOMES[key] ?? 0}`);

// WebAssembly only (the native driver reads text): a number argument must be a JavaScript number
// and a string argument a JavaScript string. Anything else is invalid or false, throws nothing and
// uses no id; '5', true and 1n are not coerced. The adds come first, so the valid add must get id 1.
const loose = new Module.Book();
const malformed = [
  ['add qty NaN', () => loose.add('GoodTillCancel', 'Buy', 100, NaN).status, 'invalid'],
  ['add qty undefined', () => loose.add('GoodTillCancel', 'Buy', 100, undefined).status, 'invalid'],
  ['add qty missing', () => loose.add('GoodTillCancel', 'Buy', 100).status, 'invalid'],
  ['add price -100', () => loose.add('GoodTillCancel', 'Buy', -100, 5).status, 'invalid'],
  ['add price null', () => loose.add('GoodTillCancel', 'Buy', null, 5).status, 'invalid'],
  ['add price Infinity', () => loose.add('GoodTillCancel', 'Buy', Infinity, 5).status, 'invalid'],
  ["add qty '5'", () => loose.add('GoodTillCancel', 'Buy', 100, '5').status, 'invalid'],
  ['add qty true', () => loose.add('GoodTillCancel', 'Buy', 100, true).status, 'invalid'],
  ['add qty 5n', () => loose.add('GoodTillCancel', 'Buy', 100, 5n).status, 'invalid'],
  ['add qty 1000001', () => loose.add('GoodTillCancel', 'Buy', 100, 1_000_001).status, 'invalid'],
  ['add type 1', () => loose.add(1, 'Buy', 100, 5).status, 'invalid'],
  ['add side null', () => loose.add('GoodTillCancel', null, 100, 5).status, 'invalid'],
  ['add at the limits', () => JSON.stringify(loose.add('GoodTillCancel', 'Buy', 1_000_000, 1_000_000)),
    '{"id":1,"status":"resting","restingQuantity":1000000,"trades":[]}'],
  ["modify id '1'", () => loose.modify('1', 'Buy', 100, 5).status, 'invalid'],
  ['modify price 5n', () => loose.modify(1, 'Buy', 5n, 5).status, 'invalid'],
  ['cancel true', () => loose.cancel(true), false],
  ["cancel '1'", () => loose.cancel('1'), false],
  ['cancel 1n', () => loose.cancel(1n), false],
  ['cancel 1', () => loose.cancel(1), true],
];
for (const [label, call, expected] of malformed) {
  let got;
  try {
    got = call();
  } catch (error) {
    got = `throws ${error?.name}`;
  }
  if (got !== expected) fail(`${label}: ${JSON.stringify(got)}, expected ${JSON.stringify(expected)}`);
}
loose.delete();

const [finalLevels, finalReadouts] = actual.slice(-2);
console.log(`calls: ${calls.length}, trades: ${trades}`);
console.log(`final ${finalLevels}`);
console.log(`final ${finalReadouts}`);
console.log('outcomes:');
for (const key of Object.keys(outcomes).sort()) console.log(`  ${key}: ${outcomes[key]}`);
console.log(`malformed calls checked: ${malformed.length}`);
if (failures) {
  console.error(`FAIL: ${failures} failure(s)`);
  process.exit(1);
}
console.log('PASS: WebAssembly and native outputs are identical (network read-outs finite on both sides), counts as pinned, malformed calls as expected');
