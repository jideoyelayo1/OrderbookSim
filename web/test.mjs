// Parity test: replays a seeded 20,000-call flow through the WebAssembly Book (web/dist, from
// web/build.sh) and through the native build of the same Book (orderbook_parity, from CMake) and
// requires identical output. The network read-outs are the exception: they start from rand()
// weights and see the wall clock, so they are only required to be finite on both sides.
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

const [finalLevels, finalReadouts] = actual.slice(-2);
console.log(`calls: ${calls.length}, trades: ${trades}`);
console.log(`final ${finalLevels}`);
console.log(`final ${finalReadouts}`);
console.log('outcomes:');
for (const key of Object.keys(outcomes).sort()) console.log(`  ${key}: ${outcomes[key]}`);
if (failures) {
  console.error(`FAIL: ${failures} difference(s) between the WebAssembly and native builds`);
  process.exit(1);
}
console.log('PASS: WebAssembly and native outputs are identical (network read-outs finite on both sides)');
