let scene, camera, rendered, cube;

function parentWidth(elem) {
  return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
  return elem.parentElement.clientHeight;
}

function initialize_3D() {
  scene = new THREE.Scene();
  scene.background = new THREE.Color(0xffffff);

  camera = new THREE.PerspectiveCamera(
    75,
    parentWidth(document.getElementById('3Dcube')) /
      parentHeight(document.getElementById('3Dcube')),
    0.1,
    1000
  );

  renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setSize(
    parentWidth(document.getElementById('3Dcube')),
    parentHeight(document.getElementById('3Dcube'))
  );

  document.getElementById('3Dcube').appendChild(renderer.domElement);

  const geometry = new THREE.BoxGeometry(5, 1, 4);

  var cubeMaterials = [
    new THREE.MeshBasicMaterial({ color: 0x03045e }),
    new THREE.MeshBasicMaterial({ color: 0x5d84b4 }),
    new THREE.MeshBasicMaterial({ color: 0x3565a1 }),
    new THREE.MeshBasicMaterial({ color: 0x03045e }),
    new THREE.MeshBasicMaterial({ color: 0x5d84b4 }),
    new THREE.MeshBasicMaterial({ color: 0x3565a1 })
  ];

  const material = new THREE.MeshFaceMaterial(cubeMaterials);

  cube = new THREE.Mesh(geometry, material);
  scene.add(cube);
  camera.position.z = 5;
  renderer.render(scene, camera);
}

function onWindowResize() {
  camera.aspect =
    parentWidth(document.getElementById('3Dcube')) /
    parentHeight(document.getElementById('3Dcube'));
  //camera.aspect = window.innerWidth /  window.innerHeight;
  camera.updateProjectionMatrix();
  //renderer.setSize(window.innerWidth, window.innerHeight);
  renderer.setSize(
    parentWidth(document.getElementById('3Dcube')),
    parentHeight(document.getElementById('3Dcube'))
  );
}

window.addEventListener('resize', onWindowResize, false);

initialize_3D();

if (!!window.EventSource) {
  var source = new EventSource('/events');

  source.addEventListener(
    'open',
    function (e) {
      console.log('Events Connected');
    },
    false
  );

  source.addEventListener(
    'error',
    function (e) {
      if (e.target.readyState != EventSource.OPEN) {
        console.log('Events Disconnected');
      }
    },
    false
  );

  source.addEventListener(
    'gyro_readings',
    function (e) {
      var obj = JSON.parse(e.data);
      document.getElementById('rotationX').innerHTML = obj.rotationX;
      document.getElementById('rotationY').innerHTML = obj.rotationY;
      document.getElementById('rotationZ').innerHTML = obj.rotationZ;

      cube.rotation.x = obj.rotationY;
      cube.rotation.z = obj.rotationX;
      cube.rotation.y = obj.rotationZ;
      renderer.render(scene, camera);
    },
    false
  );

  source.addEventListener(
    'temperature_reading',
    function (e) {
      console.log('temperature_reading', e.data);
      document.getElementById('temperature').innerHTML = e.data;
    },
    false
  );

  source.addEventListener(
    'accelerometer_readings',
    function (e) {
      console.log('accelerometer_readings', e.data);
      var obj = JSON.parse(e.data);
      document.getElementById('accelerationX').innerHTML = obj.accelerationX;
      document.getElementById('accelerationY').innerHTML = obj.accelerationY;
      document.getElementById('accelerationZ').innerHTML = obj.accelerationZ;
    },
    false
  );
}

function resetPosition(element) {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', '/' + element.id, true);
  console.log(element.id);
  xhr.send();
}
