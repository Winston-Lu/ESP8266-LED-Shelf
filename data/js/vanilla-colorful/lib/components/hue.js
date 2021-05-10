import { Interactive } from "./interactive.js?module";
import { hsvaToHslString } from "../utils/convert.js?module";
import { createTemplate, createRoot } from "../utils/dom.js?module";
import { clamp, round } from "../utils/math.js?module";
import styles from "../styles/hue.js?module";
const template = createTemplate(`<style>${styles}</style>`);
export class Hue extends Interactive {
  constructor() {
    super();
    createRoot(this, template);
    this.setAttribute('aria-label', 'Hue');
    this.setAttribute('aria-valuemin', '0');
    this.setAttribute('aria-valuemax', '360');
  }
  connectedCallback() {
    if (this.hasOwnProperty('hue')) {
      const value = this.hue;
      delete this['hue'];
      this.hue = value;
    }
  }
  get xy() {
    return false;
  }
  get hue() {
    return this._h;
  }
  set hue(h) {
    this._h = h;
    this.setStyles({
      left: `${h / 360 * 100}%`,
      color: hsvaToHslString({ h, s: 100, v: 100, a: 1 }) });

    this.setAttribute('aria-valuenow', `${round(h)}`);
  }
  getMove(interaction, key) {
    // Hue measured in degrees of the color circle ranging from 0 to 360
    return { h: key ? clamp(this.hue + interaction.left * 360, 0, 360) : 360 * interaction.left };
  }}

customElements.define('vc-hue', Hue);