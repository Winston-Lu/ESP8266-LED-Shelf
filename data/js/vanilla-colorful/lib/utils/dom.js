export const createTemplate = tpl => {
  const template = document.createElement('template');
  template.innerHTML = tpl;
  return template;
};
export const createRoot = (node, tpl) => {
  const root = node.shadowRoot || node.attachShadow({ mode: 'open' });
  root.appendChild(tpl.content.cloneNode(true));
  return root;
};