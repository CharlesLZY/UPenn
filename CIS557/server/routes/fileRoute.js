const express = require('express');
const multer = require('multer');

// router
const router = express.Router();

// database models
const FileSystem = require('../models/FileSystem');

// middleware
// handle form data, memoryStorage by default
router.use(multer({
  limits: {
    fileSize: 64 * 1024 * 1024, // 64 MB (max file size)
  },
}).single('file'));

// endpoints
router.post('/', (req, res) => {
  const { originalname, mimetype, buffer } = req.file;
  FileSystem.uploadFile(originalname, mimetype, buffer)
    .then((metadata) => {
      res.status(201).json(metadata._id);
    })
    .catch((err) => {
      console.log(err);
      res.status(500).json('Internal server error');
    });
});

router.get('/delete', async (req, res) => {
  FileSystem.clear().then(
    () => {
      res.status(200).json({ msg: 'All files are deleted.' });
    },
  );
});

router.get('/:id', (req, res) => {
  const { id } = req.params;
  FileSystem.downloadFileById(id)
    .then((downloadStream) => {
      downloadStream.pipe(res);
    })
    .catch((err) => {
      console.log(err);
      res.status(404).json('File not found');
    });
});

router.put('/:id', (req, res) => {
  const { id } = req.params;
  const { originalname, mimetype, buffer } = req.file;
  FileSystem.updateFileById(id, originalname, mimetype, buffer)
    .then((metadata) => {
      res.status(201).json(metadata);
    })
    .catch((err) => {
      console.log(err);
      res.status(500).json('Internal server error');
    });
});

router.delete('/:id', (req, res) => {
  const { id } = req.params;
  FileSystem.deleteFileById(id)
    .then(() => {
      res.status(200).json(`File with id <${id}> delete success`);
    })
    .catch((err) => {
      console.log(err);
      res.status(404).send('File not found');
    });
});

// export
module.exports = router;
