let array = [
  [1, 2, 16, 5, 13],
  [1, 13, 9, 10, 14],
  [1, 14, 6, 15, 2],
  [2, 15, 11, 12, 16],
  [3, 4, 18, 8, 17],
  [3, 17, 12, 11, 20],
  [3, 20, 7, 19, 4],
  [19, 10, 9, 18, 4],
  [16, 12, 17, 8, 5],
  [5, 8, 18, 9, 13],
  [14, 10, 19, 7, 6],
  [6, 7, 20, 11, 15],
];

array = array.map((elem) => elem.map(term => term-1));


const fs = require("fs");
fs.writeFileSync("./file.log",JSON.stringify(array))