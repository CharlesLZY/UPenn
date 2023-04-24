import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { FixedSizeList } from 'react-window';

import { makeStyles } from '@mui/styles';
import {
  Paper, Tab, Tabs, Button, Typography, Modal,
} from '@mui/material';

import Close from '@mui/icons-material/Close';
import Check from '@mui/icons-material/Check';

import PostPreview from './PostPreview';

import * as fetchGroup from '../../../utils/fetchPrivateGroup';
import * as fetchPost from '../../../utils/fetchPrivatePost';

const useStyles = makeStyles({
  requestSectionBG: {
    marginTop: 10,
    width: 550,
    paddingBottom: 10,
  },

  fixedSizeList: {
    marginLeft: 'auto',
    marginRight: 'auto',
    marginBottom: 20,
    marginTop: 10,
  },

  requestCard: {
    width: 500,
    marginLeft: 'auto',
    marginRight: 'auto',
  },

  requestCardBG: {
    paddingTop: 5,
    paddingBottom: 5,
  },

  requestContent: {
    display: 'flex',
    justifyContent: 'center',
  },

  postLink: {
    '&:hover': {
      color: '#F5AF64',
      cursor: 'pointer',
    },
  },

  modal: {
    margin: 0,
    top: '15%',
    right: document.body.clientWidth / 2 - 250,
    bottom: 'auto',
    left: 'auto',
    position: 'fixed',
  },

});

export default function RequestBar({ group }) {
  const classes = useStyles();
  const navigate = useNavigate();
  const [value, setValue] = useState(0);
  const [postModal, setPostModal] = useState(false);
  const [post, setPost] = useState(null);

  const handleClick = (e, val) => {
    setValue(val);
  };

  function TabPanel(props) {
    const { children, value, index } = props;
    return (
      <div>
        {value === index && <div>{children}</div>}
      </div>
    );
  }

  const handleJoinYes = async (userToJoin) => {
    await fetchGroup.approveRequestJoin(group.id, userToJoin);
    navigate(0);
  };

  const handleJoinNo = async (userToJoin) => {
    await fetchGroup.removeRequestJoin(group.id, userToJoin);
    navigate(0);
  };

  function joinRequests() {
    const requests = group.joinRequest;
    return (

      requests.map((userToJoin) => (
        <div className={classes.requestCardBG} key={`${userToJoin}_toJoin`}>
          <Paper className={classes.requestCard} elevation={2}>
            <div className={classes.requestContent}>
              <Typography color="secondary" variant="h6">
                {userToJoin}
              </Typography>
              <Typography variant="h6">
                            &thinsp; want to join &thinsp;
              </Typography>
              <Typography variant="h6" color="secondary">
                {group.id}
              </Typography>
              <Typography variant="h6">
                .
              </Typography>
            </div>
            <div className={classes.requestContent}>
              <Button
                startIcon={<Check />}
                sx={{
                  marginTop: '5px', marginLeft: '50px', marginRight: '50px', color: '#000000',
                }}
                onClick={() => handleJoinYes(userToJoin)}
              >
                Yes
              </Button>
              <Button
                startIcon={<Close />}
                sx={{
                  marginTop: '5px', marginLeft: '50px', marginRight: '50px', color: '#000000',
                }}
                onClick={() => handleJoinNo(userToJoin)}
              >
                No
              </Button>
            </div>
          </Paper>
        </div>
      ))
    );
  }

  const handleClickLink = async (postID) => {
    setPostModal(true);
    const res = await fetchPost.fetchPost(postID);
    setPost(res);
  };

  const handleClickDelYes = async (deletionReq) => {
    await fetchGroup.deletePost(deletionReq.postID, group.id);
    await fetchGroup.unflagForDeletion(group.id, deletionReq.from, deletionReq.postID);
    navigate(0);
  };

  const handleClickDelNo = async (deletionReq) => {
    await fetchGroup.unflagForDeletion(group.id, deletionReq.from, deletionReq.postID);
    await fetchPost.unflagForDeletion(deletionReq.postID, deletionReq.from);
    navigate(0);
  };

  function deletionRequests() {
    const requests = group.deletionRequest;
    return (

      requests.map((deletionReq) => (
        <div className={classes.requestCardBG} key={`${deletionReq.from}_del_${deletionReq.postID}`}>
          <Paper className={classes.requestCard} elevation={2}>
            <div className={classes.requestContent}>
              <Typography color="secondary" variant="h6">
                {deletionReq.from}
              </Typography>
              <Typography variant="h6">
                                &thinsp; flaged a &thinsp;
              </Typography>
              <Typography variant="h6" color="secondary" className={classes.postLink} onClick={() => handleClickLink(deletionReq.postID)}>
                post
              </Typography>
              <Typography variant="h6">
                                &thinsp; for deletion.
              </Typography>

            </div>
            <div className={classes.requestContent}>
              <Button
                startIcon={<Check />}
                sx={{
                  marginTop: '5px', marginLeft: '50px', marginRight: '50px', color: '#000000',
                }}
                onClick={() => handleClickDelYes(deletionReq)}
              >
                Yes
              </Button>
              <Button
                startIcon={<Close />}
                sx={{
                  marginTop: '5px', marginLeft: '50px', marginRight: '50px', color: '#000000',
                }}
                onClick={() => handleClickDelNo(deletionReq)}
              >
                No
              </Button>
            </div>
          </Paper>
        </div>
      ))
    );
  }

  return (
    <div>
      <Paper className={classes.requestSectionBG}>
        <Tabs
          value={value}
          variant="fullWidth"
          TabIndicatorProps={{ style: { background: '#ffffff' } }}
          onChange={handleClick}
        >
          <Tab label="Join Request" />
          <Tab label="Deletion Request" />
        </Tabs>
        <TabPanel value={value} index={0}>
          <FixedSizeList
            height={420}
            width={550}
            itemSize={50}
            itemCount={1}
            className={classes.fixedSizeList}
          >
            {joinRequests}
          </FixedSizeList>

        </TabPanel>
        <TabPanel value={value} index={1}>
          <FixedSizeList
            height={420}
            width={550}
            itemSize={50}
            itemCount={1}
            className={classes.fixedSizeList}
          >
            {deletionRequests}
          </FixedSizeList>

        </TabPanel>
      </Paper>
      <Modal open={postModal} onClose={() => setPostModal(false)}>
        <div className={classes.modal}>
          <Paper>
            <PostPreview post={post} setPostModal={setPostModal} />
          </Paper>
        </div>
      </Modal>
    </div>
  );
}
