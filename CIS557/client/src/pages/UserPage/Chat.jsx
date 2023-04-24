import React, { useState, useEffect, useRef } from 'react';
import { useNavigate } from 'react-router-dom';

import { makeStyles } from '@mui/styles';
import {
  Paper, Typography, Avatar, Divider, IconButton, Button, TextField, Modal,
} from '@mui/material';

import ListItemButton from '@mui/material/ListItemButton';
import ListItemText from '@mui/material/ListItemText';
import ListItemAvatar from '@mui/material/ListItemAvatar';

import ImageIcon from '@mui/icons-material/Image';
import AudioIcon from '@mui/icons-material/GraphicEq';
import VideoIcon from '@mui/icons-material/Videocam';

import DeleteIcon from '@mui/icons-material/Delete';
import ArrowUpward from '@mui/icons-material/ArrowUpward';

import { io } from 'socket.io-client';
import MessageBubble from '../../components/UserPage/MessageBubble';

import * as fetchUser from '../../utils/fetchUser';
import * as fetchMessage from '../../utils/fetchMessage';
import * as utils from '../../utils/utils';

import Layout from '../../components/Layout/Layout';

const useStyles = makeStyles({
  pos: {
    display: 'flex',
    justifyContent: 'center',
    paddingBottom: 50,
  },

  root: {
    display: 'flex',
    width: 1100,
    height: 671,
    marginTop: 50,
  },

  sideBar: {
    borderRight: '1px solid #e1e1e1',
  },

  chatList: {

    height: 628,
    width: 300,
    overflow: 'auto',
    borderLeft: '1px solid #e1e1e1',

  },

  chat: {
    width: 800,
  },

  title: {
    height: 32,
    paddingTop: 5,
    paddingBottom: 5,
    borderBottom: '1px solid #e1e1e1',
    display: 'flex',
    justifyContent: 'space-between',
  },

  chatBox: {
    borderTop: '1px solid #e1e1e1',
    backgroundColor: '#f6f6f6',
    height: 450,
    overflow: 'auto',
    borderRight: '1px solid #e1e1e1',
  },

  btnArea: {
    display: 'flex',
    height: 40,
    marginTop: 5,
  },

  inputBar: {
    borderRight: '1px solid #e1e1e1',

  },

  padding: {
    width: 20,
  },

  modal: {
    paddingTop: 50,
    width: 500,
    height: 100,
    textAlign: 'center',
    margin: 0,
    top: '35%',
    right: document.body.clientWidth / 2 - 200,
    bottom: 'auto',
    left: 'auto',
    position: 'fixed',
  },

});

export default function Chat() {
  const classes = useStyles();
  const navigate = useNavigate();

  const messageEl = useRef(null);
  const [selectedIndex, setSelectedIndex] = useState(null);
  const [chatTo, setChatTo] = useState(null);
  const [infoModal, setInfoModal] = useState(false);
  const [info, setInfo] = useState('');

  const [userChat, setUserChat] = useState(null);
  const [currentChat, setCurrentChat] = useState([]);

  const userID = sessionStorage.getItem('userID');

  const [text, setText] = useState('');
  const [textError, setTextError] = useState(false);
  const textInput = useRef(null);

  const [file, setFile] = useState(null);
  const [fileType, setFileType] = useState(null);
  const hiddenImageRef = useRef(null);
  const hiddenAudioRef = useRef(null);
  const hiddenVideoRef = useRef(null);

  const [newMessage, setNewMessage] = useState(null);

  const socket = useRef(io());

  useEffect(async () => {
    if (selectedIndex) {
      const data = await fetchMessage.fetchChat(selectedIndex);
      data.sort((a, b) => a.date - b.date);
      setCurrentChat(data);
    }
  }, [selectedIndex]);

  useEffect(async () => {
    socket.current = io();
    socket.current.on('getMessage', async (data) => {
      const message = await fetchMessage.fetchMessage(data.messageID);
      setNewMessage(message);
    });
  }, []);

  useEffect(() => {
    if (newMessage) {
      if (newMessage.chatID === selectedIndex) {
        setCurrentChat(currentChat.concat([newMessage]));
      }
    }
  }, [newMessage]);

  useEffect(() => {
    socket.current.emit('select', selectedIndex);
  }, [selectedIndex]);

  useEffect(async () => {
    if (!userID) {
      navigate('/login');
    } else {
      const chats = await fetchUser.fetchChat(userID);
      chats.sort((a, b) => b.last_date - a.last_date);
      setUserChat(chats);
    }
  }, []);

  useEffect(() => {
    if (messageEl) {
      messageEl.current.addEventListener('DOMNodeInserted', (event) => {
        const { currentTarget: target } = event;
        target.scroll({ top: target.scrollHeight, behavior: 'auto' });
      });
    }
  }, [currentChat]);

  useEffect(async () => {
    if (file) {
      const messageID = await fetchMessage.sendMessage(userID, chatTo, null, file, fileType, selectedIndex);
      // const data = await fetchMessage.fetchChat(selectedIndex);
      // data.sort((a,b) => a.date - b.date);
      // setCurrentChat(data);

      setFile(null);
      setFileType(null);

      socket.current.emit('newMessage', { chatID: selectedIndex, id: messageID });
    }
  }, [file]);

  const handleClickDelete = async () => {
    await fetchMessage.deleteChat(userID, selectedIndex);
    navigate(0);
  };

  const handleSelect = (chat) => {
    setSelectedIndex(chat.chatID);
    setChatTo(chat.chatTo);
  };

  const handleSubmit = async () => {
    if (text === '') {
      setTextError(true);
      setInfo('Message can not be empty.');
      setInfoModal(true);
    } else {
      let res = await fetchMessage.check(userID, chatTo);
      if (res !== 200) {
        setInfo(`You can not send message to ${chatTo}.`);
        setInfoModal(true);
      } else {
        
        res = await fetchMessage.sendMessage(userID, chatTo, text, null, 'text', selectedIndex);
        if (res === 404) {
          setInfo('Error');
          setInfoModal(true);
        } else {
          // const data = await fetchMessage.fetchChat(selectedIndex);
          // data.sort((a,b) => a.date - b.date)
          // setCurrentChat(data)
          socket.current.emit('newMessage', { chatID: selectedIndex, id: res });
        }
      }

      textInput.current.value = '';
      setText('');
    }
  };

  const handleClickImage = async () => {
    hiddenImageRef.current.click();
  };

  const handleClickAudio = async () => {
    hiddenAudioRef.current.click();
  };

  const handleClickVideo = async () => {
    hiddenVideoRef.current.click();
  };

  function renderRow(chatList) {
    if (chatList) {
      return (
        <div>
          <Divider />
          {chatList.map((chat) => (
            <div key={chat.chatID}>
              <ListItemButton selected={selectedIndex === chat.chatID} onClick={() => handleSelect(chat)}>
                <ListItemAvatar>
                  <Avatar />
                </ListItemAvatar>
                <ListItemText primary={chat.chatTo} sx={{ width: '100px' }} />

              </ListItemButton>
              <Divider />
            </div>
          ))}
        </div>
      );
    }
  }

  function renderMessages(thread) {
    return (
      thread.map((message) => <MessageBubble key={`${message.date}${Math.random()}`} message={message} self={userID} />)
    );
  }

  return (
    <Layout>
      <div className={classes.pos}>
        <Paper className={classes.root}>
          <div className={classes.sideBar}>
            <div className={classes.title}>
              <div />
              <Typography variant="h6" align="center" sx={{ marginLeft: '20px' }}>
                Recent Contact
              </Typography>
              <div />
            </div>
            <div className={classes.chatList}>
              {renderRow(userChat)}
            </div>
          </div>
          <div className={classes.chat}>
            <div className={classes.title}>
              <div className={classes.padding} />
              <Typography variant="h6" align="center">
                {chatTo}
              </Typography>
              {
                selectedIndex
                  ? (
                    <IconButton size="small" onClick={handleClickDelete}>
                      <DeleteIcon />
                    </IconButton>
                  )
                  : <div />
              }
            </div>
            <div className={classes.chatBox} ref={messageEl}>
              {renderMessages(currentChat)}
            </div>
            <div className={classes.inputBar}>
              {selectedIndex
                ? (
                  <div className={classes.btnArea}>
                    <label htmlFor="uploadImage">
                      <IconButton
                        onClick={handleClickImage}
                      >
                        <ImageIcon />
                      </IconButton>
                    </label>

                    <label htmlFor="uploadAudio">
                      <IconButton
                        onClick={handleClickAudio}
                      >
                        <AudioIcon />
                      </IconButton>
                    </label>
                    <label htmlFor="uploadVideo">
                      <IconButton
                        onClick={handleClickVideo}
                      >
                        <VideoIcon />
                      </IconButton>
                    </label>
                    <Button
                      endIcon={<ArrowUpward />}
                      sx={{
                        marginLeft: '600px', color: '#000000', backgroundColor: '#ffffff', '&:hover': { background: '#f5f5f5' },
                      }}
                      onClick={handleSubmit}
                    >
                      Send
                    </Button>
                  </div>
                )
                : <div className={classes.btnArea} />}
              <TextField
                sx={{ marginTop: '7px' }}
                multiline
                rows={4}
                fullWidth
                color="secondary"
                id="inputBar"
                disabled={selectedIndex === null}
                type="text"
                error={textError}
                inputRef={textInput}
                inputProps={{ maxLength: 530 }}
                onChange={(e) => {
                  setText(e.target.value);
                  setTextError(false);
                }}
              />
            </div>
          </div>

        </Paper>
        <Modal open={infoModal} onClose={() => setInfoModal(false)}>
          <Paper className={classes.modal}>
            <Typography variant="h6">
              {info}
            </Typography>
          </Paper>
        </Modal>

        <input
          id="uploadImage"
          style={{ display: 'none' }}
          type="file"
          ref={hiddenImageRef}
          onChange={
            (e) => {
              const file = e.target.files[0];
              if (utils.checkImage(file)) {
                setFileType('image');
                setFile(file);
              } else if (file) {
                setInfoModal(true);
                setInfo('Invalid File. Only accept JPG/JEPG/PNG/GIF.');
              }
            }
          }
        />

        <input
          id="uploadAudio"
          style={{ display: 'none' }}
          type="file"
          ref={hiddenAudioRef}
          onChange={
            (e) => {
              const file = e.target.files[0];
              if (utils.checkAudio(file)) {
                setFileType('audio');
                setFile(file);
              } else if (file) {
                setInfoModal(true);
                setInfo('Invalid File. Only accept MP3(<= 15MB).');
              }
            }
          }
        />

        <input
          id="uploadVideo"
          style={{ display: 'none' }}
          type="file"
          ref={hiddenVideoRef}
          onChange={
            (e) => {
              const file = e.target.files[0];
              if (utils.checkVideo(file)) {
                setFileType('video');
                setFile(file);
              } else if (file) {
                setInfoModal(true);
                setInfo('Invalid File. Only accept MP4(<= 15MB).');
              }
            }
          }
        />

      </div>
    </Layout>
  );
}
