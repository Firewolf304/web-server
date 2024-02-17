function parseJSON(json) {
    let output = '';
    const data = JSON.parse(json);

    function buildOutput(obj, indent) {
        for (let key in obj) {
            if (typeof obj[key] === 'object') {
                output += `${indent}${key}:\n`;
                if (Array.isArray(obj[key])) {
                    obj[key].forEach((item) => {
                        output += `${indent}\t${item}\n`;
                    });
                } else {
                    buildOutput(obj[key], `${indent}\t`);
                }
            } else {
                output += `${indent}${key}=${obj[key]}\n`;
            }
        }
    }

    buildOutput(data, '');

    return output;
}

const json = '{"example": {"123":"123", "23":"23"}, "example1":["1","2","3"], "example2":"1"}';
const parsedJSON = parseJSON(json);
console.log(parsedJSON);