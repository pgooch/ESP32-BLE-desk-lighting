const { menubar } = require( 'menubar' );
const mb = menubar({
  dir: './src/',
  browserWindow: {
    width: 300,
    height: 500
  }
});

mb.on('after-create-window',()=>{
  //mb.window.openDevTools();
});