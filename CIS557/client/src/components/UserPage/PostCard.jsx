import React from 'react';
import { useNavigate } from 'react-router-dom';
import Card from '@mui/material/Card';
import CardHeader from '@mui/material/CardHeader';
import CardMedia from '@mui/material/CardMedia';
import CardContent from '@mui/material/CardContent';
import CardActionArea from '@mui/material/CardActionArea';
import CardActions from '@mui/material/CardActions';
import { Button, Typography, IconButton } from '@mui/material';

import Forum from '@mui/icons-material/Forum';
import Share from '@mui/icons-material/Share';
import Delete from '@mui/icons-material/Delete';

import * as fetchUser from '../../utils/fetchUser';
import * as utils from '../../utils/utils';
import { baseURL } from '../../utils/utils';

export default function PostCard({ post }) {
  const navigate = useNavigate();
  const cardStyle = {
    display: 'block',
    width: '800px',
    marginBottom: '25px',
  };

  const handleClick = (post) => {
    navigate(`/${post.type}/${post._id}`);
  };

  const handleClickDelete = () => {
    fetchUser.deletePost(post._id, post.type);
    navigate(0);
  };

  return (
    <div>
      <Card style={cardStyle} elevation={3}>

        <CardHeader
          action={<IconButton onClick={handleClickDelete}><Delete /></IconButton>}
          title={post.title}
          subheader={`${utils.convertTime(post.date)} Written by ${post.author} from ${post.group}`}
        />
        <CardActionArea onClick={() => handleClick(post)}>
          <CardContent>
            <Typography variant="h6">
              {post.content}
            </Typography>
          </CardContent>
          <CardMedia
            component={utils.parseFileType(post.attachment.fileType)}
            src={post.attachment.fileID ? `${baseURL}/file/${post.attachment.fileID}` : null}
            controls
          />
        </CardActionArea>
        <CardActions>
          <Button size="small" sx={{ color: '#000000', '&:hover': { background: '#f5f5f5' } }} onClick={() => handleClick(post)} startIcon={<Forum />}>
            {post.comments.length}
            {' '}
            Comments
          </Button>

          <Button size="small" sx={{ color: '#000000', '&:hover': { background: '#f5f5f5' } }} startIcon={<Share />}>
            Share
          </Button>
        </CardActions>

      </Card>
    </div>
  );
}
