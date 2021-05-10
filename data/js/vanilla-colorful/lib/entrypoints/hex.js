import { ColorPicker } from "../components/color-picker.js?module";
import { hexToHsva, hsvaToHex } from "../utils/convert.js?module";
import { equalHex } from "../utils/compare.js?module";
const colorModel = {
  defaultColor: '#000',
  toHsva: hexToHsva,
  fromHsva: hsvaToHex,
  equal: equalHex,
  fromAttr: color => color };

export class HexBase extends ColorPicker {
  get colorModel() {
    return colorModel;
  }}