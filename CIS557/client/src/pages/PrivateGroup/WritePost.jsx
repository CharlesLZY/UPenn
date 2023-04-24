import React, { useState, useEffect, useRef } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Typography, Button, TextField, Paper, Container, IconButton,
} from '@mui/material';

import ArrowUpward from '@mui/icons-material/ArrowUpward';
import AttachFile from '@mui/icons-material/AttachFile';
import Close from '@mui/icons-material/Close';
import Layout from '../../components/Layout/Layout';

import * as fetchGroup from '../../utils/fetchPrivateGroup';
import * as upload from '../../utils/uploadPost';
import * as utils from '../../utils/utils';

const useStyles = makeStyles({
  paper: {
    display: 'inlined',
    width: '850px',
    marginLeft: 'auto',
    marginRight: 'auto',
  },
  padding: {
    height: '20vw',
  },
  close: {
    marginTop: '5vw',
    width: '850px',
    marginLeft: 'auto',
    marginRight: 'auto',
  },
  closeButton: {
    marginLeft: 820,
  },

  titleBar: {
    marginBottom: 6,
  },

  infoArea: {
    width: '850px',
    marginTop: '1vw',
    marginLeft: 'auto',
    marginRight: 'auto',
  },

  buttonArea: {
    width: '850px',
    marginTop: '1vw',
    marginLeft: 'auto',
    marginRight: 'auto',
    display: 'flex',
  },

  info: {
    paddingTop: 6,

  },
});

export default function WritePost() {
  const classes = useStyles();
  const params = useParams();
  const navigate = useNavigate();
  const { groupID } = params;
  const userID = sessionStorage.getItem('userID');

  const [title, setTitle] = useState('');
  const [titleError, setTitleError] = useState(false);
  const [text, setText] = useState('');
  const [textError, setTextError] = useState(false);
  const [file, setFile] = useState(null);
  const [info, setInfo] = useState('');
  const titleInput = useRef(null);
  const textInput = useRef(null);
  const hiddenFileRef = useRef(null);

  const handleSubmit = async () => {
    if (title === '') {
      setTitleError(true);
    }
    if (text === '') {
      setTextError(true);
    }

    if (title !== '' && text !== '') {
      const data = {
        group: groupID,
        author: userID,
        title,
        content: text,
        file,
      };

      await upload.uploadPrivatePost(data);
      setFile(null);
      setTitle('');
      setText('');
      navigate(`/privategroup/${groupID}`);
    }
  };

  const handleAttachment = () => {
    hiddenFileRef.current.click();
  };

  const handleClose = () => {
    navigate(`/privategroup/${groupID}`);
  };

  useEffect(async () => {
    if (!userID) {
      navigate('/login');
    } else {
      const data = await fetchGroup.fetchGroup(groupID);
      if (data === null) {
        navigate('/wrongpage');
      } else if (data.members.indexOf(userID) < 0) {
        navigate('/permission');
      } else if (data.members.indexOf(userID) < 0) {
        navigate(`/privategroup/${groupID}`);
      }
    }
  }, []);

  return (
    <Layout>
      <Container>
        <div className={classes.close}>
          <IconButton className={classes.closeButton} onClick={handleClose}><Close /></IconButton>
        </div>
        <Paper className={classes.paper} elevation={1}>
          <div className={classes.titleBar}>
            <TextField
              id="titleBar"
              variant="outlined"
              color="secondary"
              label="Title"
              multiline
              rows={1}
              fullWidth
              type="text"
              inputRef={titleInput}
              error={titleError}
              inputProps={{ maxLength: 80 }}
              onChange={(e) => {
                setTitle(e.target.value);
                setTitleError(false);
              }}
            />
          </div>
        </Paper>
        <Paper className={classes.paper} elevation={1}>
          <div className={classes.inputBar}>
            <TextField
              id="inputBar"
              variant="outlined"
              color="secondary"
              label={`Write your post in ${groupID}`}
              multiline
              rows={10}
              fullWidth
              type="text"
              inputRef={textInput}
              error={textError}
              inputProps={{ maxLength: 2000 }}
              onChange={(e) => {
                setText(e.target.value);
                setTextError(false);
              }}
            />
          </div>
        </Paper>
        <div className={classes.buttonArea}>

          <Button color="primary" variant="contained" startIcon={<ArrowUpward />} onClick={handleSubmit}> Post</Button>
          <input
            id="upload"
            style={{ display: 'none' }}
            type="file"
            ref={hiddenFileRef}
            onChange={
              (e) => {
                const file = e.target.files[0];
                if (utils.checkFile(file)) {
                  setFile(file);
                  setInfo('');
                } else {
                  setFile(null);
                  setInfo('Invalid attachment. Only accept JPG/JEPG/PNG/GIF/MP3/MP4(<= 30MB).');
                }
              }
            }
          />
          <label htmlFor="upload">
            <IconButton onClick={handleAttachment}><AttachFile /></IconButton>
          </label>
          <Typography className={classes.info}>
            {file ? `${file.name} is attached.` : ''}
          </Typography>

        </div>
        <div className={classes.infoArea}>
          <Typography>
            {info}
          </Typography>
        </div>
        <div className={classes.padding} />
      </Container>
    </Layout>
  );
}
