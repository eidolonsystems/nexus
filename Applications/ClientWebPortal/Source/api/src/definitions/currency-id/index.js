class CurrencyId {
  constructor(number) {
    this.value = Number(number);
    if (isNaN(this.value)) {
      throw new Error('Input must be a number.');
    } else if (this.value % 1 != 0) {
      throw new Error('Input must be an integer.');
    }
  }

  equals(operand) {
    return this.value === operand.value;
  }

  toNumber() {
    return this.value;
  }

  toData() {
    return this.value;
  }

  toString() {
    return this.value;
  }

  clone() {
    return CurrencyId.fromData(this.value);
  }
}

CurrencyId.fromData = (number) => {
  return new CurrencyId(number);
};

export default CurrencyId;
