import { Interactive } from "./interactive.js?module";
import { hsvaToHslString } from "../utils/convert.js?module";
import { createTemplate, createRoot } from "../utils/dom.js?module";
import { clamp, round } from "../utils/math.js?module";
import styles from "../styles/saturation.js?module";
const template = createTemplate(`<style>${styles}</style>`);
export class Saturation extends Interactive {
  constructor() {
    super();
    createRoot(this, template);
    this.setAttribute('aria-label', 'Color');
  }
  connectedCallback() {
    if (this.hasOwnProperty('hsva')) {
      const value = this.hsva;
      delete this['hsva'];
      this.hsva = value;
    }
  }
  get xy() {
    return true;
  }
  get hsva() {
    return this._hsva;
  }
  set hsva(hsva) {
    this._hsva = hsva;
    this.style.backgroundColor = hsvaToHslString({ h: hsva.h, s: 100, v: 100, a: 1 });
    this.setStyles({
      top: `${100 - hsva.v}%`,
      left: `${hsva.s}%`,
      color: hsvaToHslString(hsva) });

    this.setAttribute('aria-valuetext', `Saturation ${round(hsva.s)}%, Brightness ${round(hsva.v)}%`);
  }
  getMove(interaction, key) {
    // Saturation and brightness always fit into [0, 100] range
    return {
      s: key ? clamp(this.hsva.s + interaction.left * 100, 0, 100) : interaction.left * 100,
      v: key ?
      clamp(this.hsva.v - interaction.top * 100, 0, 100) :
      Math.round(100 - interaction.top * 100) };

  }}

customElements.define('vc-saturation', Saturation);