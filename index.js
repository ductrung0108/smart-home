import { initializeApp } from 'https://www.gstatic.com/firebasejs/10.7.1/firebase-app.js';
import { getDatabase, ref, set, onValue } from 'https://www.gstatic.com/firebasejs/10.7.1/firebase-database.js';

setTimeout(() => {
  document.getElementById('loading').style.display = 'none';
}, 2000);

const firebaseConfig = {
  apiKey: "AIzaSyCH7NojcKtJIG98LKan1WxfFsDIn1bNe9A",
  authDomain: "smart-home-d65f6.firebaseapp.com",
  databaseURL: "https://smart-home-d65f6-default-rtdb.firebaseio.com",
  projectId: "smart-home-d65f6",
  storageBucket: "smart-home-d65f6.appspot.com",
  messagingSenderId: "658367806697",
  appId: "1:658367806697:web:c866103d0ba4e2b2658478"
}
const app = initializeApp(firebaseConfig);

const lightCheckbox = document.getElementById('light');
const luminosityRange = document.getElementById('luminosity');
const lockCheckbox = document.getElementById('lock');
const watersprayCheckbox = document.getElementById('waterspray');
const windowCheckbox = document.getElementById('window');
const gasReading = document.getElementById('gas');

const lightUI = document.getElementById('lightUI');
const luminosityUI = document.getElementById('luminosityUI');
const lockUI = document.getElementById('lockUI');
const watersprayUI = document.getElementById('watersprayUI');
const windowUI = document.getElementById('windowUI');

const database = getDatabase(app);

const lightref = ref(database, 'light');
const windowref = ref(database, 'window');
const luminosityref = ref(database, 'luminosity');
const gasref = ref(database, 'gas');
const watersprayref = ref(database, 'waterspray');
const lockref = ref(database, 'lock');


onValue(lightref, (snapshot) => {
  const data = snapshot.val();
  lightCheckbox.checked = data;
});

onValue(lockref, (snapshot) => {
  const data = snapshot.val();
  lockCheckbox.checked = data;
});

onValue(windowref, (snapshot) => {
  const data = snapshot.val();
  windowCheckbox.checked = data;
});

onValue(watersprayref, (snapshot) => {
  const data = snapshot.val();
  watersprayCheckbox.checked = data;
});

onValue(gasref, (snapshot) => {
  const data = snapshot.val();
  gasReading.innerHTML = data;
});

onValue(luminosityref, (snapshot) => {
  const data = snapshot.val();
  luminosityRange.value = data;
});

lightUI.onclick = () => {set(lightref, !lightCheckbox.checked)}
luminosityRange.oninput = () => {set(luminosityref, luminosityRange.value)}
windowUI.onclick = () => {set(windowref, !windowCheckbox.checked)}
lockUI.onclick = () => {set(lockref, !lockCheckbox.checked)}
watersprayUI.onclick = () => {set(watersprayref, !watersprayCheckbox.checked)}









