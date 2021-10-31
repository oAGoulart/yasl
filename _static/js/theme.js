document.addEventListener('DOMContentLoaded', () => {
  var palette = localStorage.getItem('palette');
  if (!palette)
    palette = window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark-palette' : 'light-palette';
  document.documentElement.className = palette;

  const lightSwitch = document.getElementById('light-switch');
  if (lightSwitch) {
    lightSwitch.addEventListener('click', () => {
      var target = 'light-palette';
      if (document.documentElement.className == 'light-palette')
        target = 'dark-palette';
      document.documentElement.className = target;
      localStorage.setItem('palette', target);
    });
  };
});
