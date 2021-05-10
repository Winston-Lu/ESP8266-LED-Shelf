import { equalColorObjects } from "../utils/compare.js?module";
import { createTemplate, createRoot } from "../utils/dom.js?module";
import "./hue.js?module";
import "./saturation.js?module";
import styles from "../styles/color-picker.js?module";
const tpl = createTemplate(`
<style>${styles}</style>
<vc-saturation part="saturation" exportparts="pointer: saturation-pointer"></vc-saturation>
<vc-hue part="hue" exportparts="pointer: hue-pointer"></vc-hue>
`);
const $h = Symbol('h');
const $s = Symbol('s');
const $isSame = Symbol('same');
const $color = Symbol('color');
const $hsva = Symbol('hsva');
const $change = Symbol('change');
export const $render = Symbol('render');
export class ColorPicker extends HTMLElement {
  constructor() {
    super();
    const root = createRoot(this, tpl);
    root.addEventListener('move', this);
    this[$s] = root.children[1];
    this[$h] = root.children[2];
  }
  static get observedAttributes() {
    return ['color'];
  }
  get color() {
    return this[$color];
  }
  set color(newColor) {
    if (!this[$isSame](newColor)) {
      const newHsva = this.colorModel.toHsva(newColor);
      this[$render](newHsva);
      this[$change](newColor, newHsva);
    }
  }
  connectedCallback() {
    // A user may set a property on an _instance_ of an element,
    // before its prototype has been connected to this class.
    // If so, we need to run it through the proper class setter.
    if (this.hasOwnProperty('color')) {
      const value = this.color;
      delete this['color'];
      this.color = value;
    } else
    if (!this.color) {
      this.color = this.colorModel.defaultColor;
    }
  }
  attributeChangedCallback(_attr, _oldVal, newVal) {
    const color = this.colorModel.fromAttr(newVal);
    if (!this[$isSame](color)) {
      this.color = color;
    }
  }
  handleEvent(event) {
    // Merge the current HSV color object with updated params.
    const newHsva = Object.assign({}, this[$hsva], event.detail);
    this[$render](newHsva);
    let newColor;
    if (!equalColorObjects(newHsva, this[$hsva]) &&
    !this[$isSame](newColor = this.colorModel.fromHsva(newHsva))) {
      this[$change](newColor, newHsva);
    }
  }
  [$isSame](color) {
    return this.color && this.colorModel.equal(color, this.color);
  }
  [$render](hsva) {
    this[$s].hsva = hsva;
    this[$h].hue = hsva.h;
  }
  [$change](color, hsva) {
    this[$color] = color;
    this[$hsva] = hsva;
    this.dispatchEvent(new CustomEvent('color-changed', { bubbles: true, detail: { value: color } }));
  }}